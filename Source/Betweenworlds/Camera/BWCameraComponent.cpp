// Between Worlds — Etapa 1.3 (Fundação: câmera)

#include "Camera/BWCameraComponent.h"

#include "Camera/BWCameraSettings.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogBWCamera, Log, All);

/** Nome do scalar parameter do M_CameraFade (contrato com o material — ver PASSOS_NO_EDITOR). */
static const FName FadeAmountParamName = FName(TEXT("FadeAmount"));

UBWCameraComponent::UBWCameraComponent()
{
    // Toda a atualização é por timer local; nunca Tick.
    PrimaryComponentTick.bCanEverTick = false;
}

void UBWCameraComponent::Initialize(USpringArmComponent* InArm, UCameraComponent* InCamera)
{
    Arm = InArm;
    Camera = InCamera;
}

void UBWCameraComponent::AddZoomStep(float AxisValue)
{
    if (!Settings)
    {
        return;
    }

    // Eixo positivo aproxima: roda para cima reduz o braço.
    // Se a roda ficar invertida no jogo, resolva com um modifier Negate no
    // IMC_DirectControl (sem recompilar).
    ZoomTarget = FMath::Clamp(ZoomTarget - AxisValue * Settings->ZoomStepWheel,
        Settings->MinArmLength, Settings->MaxArmLength);
}

void UBWCameraComponent::SetZoomAxisInput(float AxisValue)
{
    ZoomAxisInput = AxisValue;
}

void UBWCameraComponent::BeginPlay()
{
    Super::BeginPlay();

    if (!Settings)
    {
        UE_LOG(LogBWCamera, Warning,
            TEXT("%s: BWCameraSettings não configurado. A câmera fica como está (sem zoom nem fade). "
                "Atribua o DA_BWCameraSettings ao componente BWCamera no BP_BWPlayerCharacter."),
            *GetNameSafe(GetOwner()));
        return;
    }

    // Aplica o Data Asset por cima do que estiver no BP (fonte única de verdade).
    if (Arm)
    {
        // O braço usa rotação absoluta; SetWorldRotation dá o pitch aprovado
        // independentemente da rotação do personagem.
        Arm->SetWorldRotation(FRotator(Settings->Pitch, 0.0f, 0.0f));
        Arm->TargetArmLength = Settings->DefaultArmLength;
        Arm->SocketOffset = Settings->SocketOffset;
    }

    CurrentArmLength = Settings->DefaultArmLength;
    ZoomTarget = Settings->DefaultArmLength;

    // O gate IsLocallyControlled fica DENTRO de UpdateCamera (reavaliado a cada
    // ciclo) porque no cliente a posse do pawn pode chegar depois do BeginPlay.
    // O custo para pawns remotos é uma comparação por ciclo.
    if (UWorld* World = GetWorld())
    {
        UpdateInterval = FMath::Max(Settings->FadeTraceInterval * 0.5f, 0.016f);
        World->GetTimerManager().SetTimer(UpdateTimerHandle,
            FTimerDelegate::CreateUObject(this, &ThisClass::UpdateCamera),
            UpdateInterval, /*bLoop*/ true);
    }
}

void UBWCameraComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }

    // Restaura sem interpolar (estamos saindo; o mundo pode estar sendo destruído).
    for (const FBWFadedPrimitive& Entry : FadedPrimitives)
    {
        if (UPrimitiveComponent* Component = Entry.Component.Get())
        {
            RestoreMaterials(Component, Entry);
        }
    }
    FadedPrimitives.Empty();

    Super::EndPlay(EndPlayReason);
}

void UBWCameraComponent::UpdateCamera()
{
    // Gate de rede/local: só a máquina que controla este pawn executa câmera.
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn || !OwnerPawn->IsLocallyControlled() || !Settings)
    {
        return;
    }

    if (!bLoggedActivation)
    {
        bLoggedActivation = true;
        UE_LOG(LogBWCamera, Log,
            TEXT("%s: câmera local ativa (zoom + fade). Em rede, cada pawn loga esta linha apenas na máquina que o controla."),
            *GetNameSafe(OwnerPawn));
    }

    UpdateZoom(UpdateInterval);
    UpdateFades(UpdateInterval);

    // Trace a cada 2 ciclos: com FadeTraceInterval = 0.1 s, update a 20 Hz e trace a 10 Hz.
    bOcclusionCycle = !bOcclusionCycle;
    if (bOcclusionCycle)
    {
        UpdateOcclusion();
    }
}

void UBWCameraComponent::UpdateZoom(float DeltaTime)
{
    if (!Arm || !Settings)
    {
        return;
    }

    // Entrada contínua (analógico) alimenta o alvo; a roda já alimentou via AddZoomStep.
    if (!FMath::IsNearlyZero(ZoomAxisInput))
    {
        ZoomTarget = FMath::Clamp(ZoomTarget - ZoomAxisInput * Settings->ZoomRateGamepad * DeltaTime,
            Settings->MinArmLength, Settings->MaxArmLength);
    }

    // Zoom suave: o braço persegue o alvo; a obstrução NÃO interfere aqui
    // (bDoCollisionTest do braço permanece desligado — decisão "não puxar a câmera").
    CurrentArmLength = FMath::FInterpTo(CurrentArmLength, ZoomTarget, DeltaTime, Settings->ZoomSmoothness);
    Arm->TargetArmLength = CurrentArmLength;
}

void UBWCameraComponent::UpdateOcclusion()
{
    if (!Arm || !Camera || !Settings)
    {
        return;
    }

    if (!Settings->FadeMaterial)
    {
        if (!bWarnedNoFadeMaterial)
        {
            bWarnedNoFadeMaterial = true;
            UE_LOG(LogBWCamera, Warning,
                TEXT("%s: FadeMaterial não configurado no BWCameraSettings. O fade de obstruções fica desligado; o zoom continua funcionando."),
                *GetNameSafe(GetOwner()));
        }
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const FVector TraceStart = Arm->GetComponentLocation(); // dobradiça do braço, acima do personagem
    const FVector TraceEnd = Camera->GetComponentLocation();
    if (FVector::DistSquared(TraceStart, TraceEnd) < FMath::Square(1.0f))
    {
        return;
    }

    FCollisionQueryParams QueryParams(TEXT("BWCameraFade"), /*bTraceComplex*/ false);
    QueryParams.AddIgnoredActor(GetOwner());

    TArray<FHitResult> Hits;
    // ATENÇÃO: em C++ o método da UWorld é LineTraceMultiByChannel (o nó de
    // Blueprint se chama "Multi Line Trace By Channel", daí a confusão comum).
    World->LineTraceMultiByChannel(Hits, TraceStart, TraceEnd, Settings->FadeTraceChannel, QueryParams);

    for (FBWFadedPrimitive& Entry : FadedPrimitives)
    {
        Entry.bSeenThisPass = false;
    }

    for (const FHitResult& Hit : Hits)
    {
        UPrimitiveComponent* HitComponent = Hit.GetComponent();
        if (!HitComponent || !HitComponent->IsVisible() || HitComponent->GetNumMaterials() == 0)
        {
            continue;
        }

        // Personagens nunca são esmaecidos (o próprio owner já está ignorado no trace).
        AActor* HitOwner = HitComponent->GetOwner();
        if (HitOwner == GetOwner() || Cast<APawn>(HitOwner))
        {
            continue;
        }

        FBWFadedPrimitive* Existing = nullptr;
        for (FBWFadedPrimitive& Entry : FadedPrimitives)
        {
            if (Entry.Component == HitComponent)
            {
                Existing = &Entry;
                break;
            }
        }

        if (!Existing)
        {
            FadedPrimitives.Add(CreateFadeEntry(HitComponent));
            Existing = &FadedPrimitives.Last();
        }

        Existing->TargetOpacity = Settings->FadeOpacity;
        Existing->bSeenThisPass = true;
    }

    // Quem não apareceu neste passe volta para opaco; UpdateFades interpola e restaura.
    for (FBWFadedPrimitive& Entry : FadedPrimitives)
    {
        if (!Entry.bSeenThisPass)
        {
            Entry.TargetOpacity = 1.0f;
        }
    }
}

void UBWCameraComponent::UpdateFades(float DeltaTime)
{
    if (!Settings)
    {
        return;
    }

    for (int32 Index = FadedPrimitives.Num() - 1; Index >= 0; --Index)
    {
        FBWFadedPrimitive& Entry = FadedPrimitives[Index];
        UPrimitiveComponent* Component = Entry.Component.Get();

        if (!Component)
        {
            // Componente destruído durante o fade: nada a restaurar.
            FadedPrimitives.RemoveAt(Index);
            continue;
        }

        Entry.CurrentOpacity = FMath::FInterpTo(Entry.CurrentOpacity, Entry.TargetOpacity,
            DeltaTime, Settings->FadeInterpSpeed);

        if (Entry.FadeMID)
        {
            Entry.FadeMID->SetScalarParameterValue(FadeAmountParamName, Entry.CurrentOpacity);
        }

        if (Entry.TargetOpacity >= 1.0f && Entry.CurrentOpacity >= 0.995f)
        {
            RestoreMaterials(Component, Entry);
            FadedPrimitives.RemoveAt(Index);
        }
    }
}

FBWFadedPrimitive UBWCameraComponent::CreateFadeEntry(UPrimitiveComponent* Component)
{
    FBWFadedPrimitive Entry;
    Entry.Component = Component;
    Entry.CurrentOpacity = 1.0f;
    Entry.TargetOpacity = 1.0f;
    Entry.bSeenThisPass = false;

    if (Settings && Settings->FadeMaterial)
    {
        // Outer = este componente; o MID morre junto com o dono da câmera.
        Entry.FadeMID = UMaterialInstanceDynamic::Create(Settings->FadeMaterial, this);
        Entry.FadeMID->SetScalarParameterValue(FadeAmountParamName, 1.0f);

        for (int32 Slot = 0; Slot < Component->GetNumMaterials(); ++Slot)
        {
            Entry.SavedMaterials.Add(Component->GetMaterial(Slot));
            Component->SetMaterial(Slot, Entry.FadeMID);
        }
    }

    return Entry;
}

void UBWCameraComponent::RestoreMaterials(UPrimitiveComponent* Component, const FBWFadedPrimitive& Entry)
{
    for (int32 Slot = 0; Slot < Entry.SavedMaterials.Num(); ++Slot)
    {
        if (Entry.SavedMaterials[Slot])
        {
            Component->SetMaterial(Slot, Entry.SavedMaterials[Slot]);
        }
    }
}

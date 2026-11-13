// Between Worlds — Etapa 1.2 (Fundação: rede) / 1.3 (câmera) / 1.4-a (clique-para-mover) / 1.4-b (InputTags) / 1.5 (GAS) / 2.5 (ataque principal + marcador de alvo)

#include "Core/BWPlayerController.h"

#include "AbilitySystem/BWAbilitySystemComponent.h"
#include "AbilitySystem/Tags/BWGameplayTags.h"
#include "Character/BWCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/DecalComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Camera/BWCameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Core/BWPlayerState.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/EngineTypes.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "Input/BWInputConfig.h"
#include "Movement/BWPathFollowComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogBWPlayerController, Log, All);

void ABWPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// No servidor existem controllers de jogadores remotos; eles não têm LocalPlayer
	// nem input. Só o controller local registra mapeamentos.
	if (!IsLocalController())
	{
		return;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!InputSubsystem)
	{
		return;
	}

	if (DirectControlMappingContext)
	{
		InputSubsystem->AddMappingContext(DirectControlMappingContext, DirectControlMappingPriority);
	}
	else
	{
		UE_LOG(LogBWPlayerController, Warning,
			TEXT("%s: DirectControlMappingContext não configurado. Defina-o no BP_BWPlayerController."),
			*GetNameSafe(this));
	}

	// Modo inicial: teclado/mouse (padrão de PC), com o IMC do clique.
	if (MouseKeyboardMappingContext)
	{
		InputSubsystem->AddMappingContext(MouseKeyboardMappingContext, MouseKeyboardMappingPriority);
	}
	else
	{
		UE_LOG(LogBWPlayerController, Warning,
			TEXT("%s: MouseKeyboardMappingContext não configurado (IMC_MouseKeyboard). O clique-para-mover não funcionará."),
			*GetNameSafe(this));
	}
	ApplyInputDeviceMode();

	// A1: troca automática de modo pelo último dispositivo físico usado.
	if (UInputDeviceSubsystem* DeviceSubsystem = UInputDeviceSubsystem::Get())
	{
		DeviceSubsystem->OnInputHardwareDeviceChanged.AddDynamic(this, &ThisClass::HandleInputHardwareDeviceChanged);
		RefreshInputDeviceModeFromHardware();
	}
}

void ABWPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UInputDeviceSubsystem* DeviceSubsystem = UInputDeviceSubsystem::Get())
	{
		DeviceSubsystem->OnInputHardwareDeviceChanged.RemoveDynamic(this, &ThisClass::HandleInputHardwareDeviceChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void ABWPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInput)
	{
		UE_LOG(LogBWPlayerController, Error,
			TEXT("InputComponent não é UEnhancedInputComponent. Verifique Project Settings > Input > Default Input Component Class."));
		return;
	}

	if (MoveAction)
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::HandleMove);

		// Started/Completed acompanham a INTENÇÃO (apertar/soltar): apertar cancela
		// caminho em curso; soltando, o clique volta a valer.
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Started, this, &ThisClass::HandleMoveStarted);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &ThisClass::HandleMoveEnded);
	}
	else
	{
		UE_LOG(LogBWPlayerController, Warning,
			TEXT("%s: MoveAction não configurada. Defina-a no BP_BWPlayerController."),
			*GetNameSafe(this));
	}

	if (ClickMoveAction)
	{
		EnhancedInput->BindAction(ClickMoveAction, ETriggerEvent::Started, this, &ThisClass::HandleClickMoveStarted);
		EnhancedInput->BindAction(ClickMoveAction, ETriggerEvent::Completed, this, &ThisClass::HandleClickMoveCompleted);
	}
	else
	{
		UE_LOG(LogBWPlayerController, Warning,
			TEXT("%s: ClickMoveAction não configurada. Defina-a (IA_ClickMove) no BP_BWPlayerController."),
			*GetNameSafe(this));
	}

	if (ZoomWheelAction)
	{
		EnhancedInput->BindAction(ZoomWheelAction, ETriggerEvent::Triggered, this, &ThisClass::HandleZoomWheel);
	}
	else
	{
		UE_LOG(LogBWPlayerController, Warning,
			TEXT("%s: ZoomWheelAction não configurada. Defina-a (IA_ZoomWheel) no BP_BWPlayerController."),
			*GetNameSafe(this));
	}

	if (ZoomAxisAction)
	{
		// Triggered alimenta enquanto o stick está inclinado; Completed zera quando solta.
		EnhancedInput->BindAction(ZoomAxisAction, ETriggerEvent::Triggered, this, &ThisClass::HandleZoomAxisChanged);
		EnhancedInput->BindAction(ZoomAxisAction, ETriggerEvent::Completed, this, &ThisClass::HandleZoomAxisEnded);
	}
	else
	{
		UE_LOG(LogBWPlayerController, Warning,
			TEXT("%s: ZoomAxisAction não configurada. Defina-a (IA_ZoomAxis) no BP_BWPlayerController."),
			*GetNameSafe(this));
	}

	// Etapa 1.4-b: ações com TAG (slots de habilidade) do BWInputConfig.
	BindTaggedInputActions(EnhancedInput);
}

// ---------------------------------------------------------------------------------
// A1: troca automática pelo último dispositivo usado.
// O UInputDeviceSubsystem da engine já rastreia o último hardware de cada usuário
// (teclado/mouse, gamepad...) e avisa quando muda. Na PIE com vários jogadores,
// cada janela tem seu próprio PlatformUserId, por isso filtramos pelo nosso.
// ---------------------------------------------------------------------------------
void ABWPlayerController::HandleInputHardwareDeviceChanged(const FPlatformUserId UserId, const FInputDeviceId /*DeviceId*/)
{
	if (!IsLocalController() || UserId != GetPlatformUserId())
	{
		return;
	}

	RefreshInputDeviceModeFromHardware();
}

void ABWPlayerController::RefreshInputDeviceModeFromHardware()
{
	UInputDeviceSubsystem* DeviceSubsystem = UInputDeviceSubsystem::Get();
	if (!DeviceSubsystem || !IsLocalController())
	{
		return;
	}

	const FHardwareDeviceIdentifier Device = DeviceSubsystem->GetMostRecentlyUsedHardwareDevice(GetPlatformUserId());
	switch (Device.PrimaryDeviceType)
	{
	case EHardwareDevicePrimaryType::Gamepad:
		SetInputDeviceMode(EBWInputDeviceMode::Gamepad);
		break;
	case EHardwareDevicePrimaryType::KeyboardAndMouse:
		SetInputDeviceMode(EBWInputDeviceMode::MouseKeyboard);
		break;
	default:
		// Tipo desconhecido (ou ainda nenhum input): mantém o modo atual.
		break;
	}
}

void ABWPlayerController::SetInputDeviceMode(EBWInputDeviceMode NewMode)
{
	if (InputDeviceMode == NewMode)
	{
		return;
	}

	InputDeviceMode = NewMode;
	ApplyInputDeviceMode();

	UE_LOG(LogBWPlayerController, Log,
		TEXT("%s: modo de entrada alterado para %s"),
		*GetNameSafe(this),
		InputDeviceMode == EBWInputDeviceMode::Gamepad ? TEXT("Controle") : TEXT("Teclado/Mouse"));

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		GetLocalPlayer() ? GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
	if (InputSubsystem && MouseKeyboardMappingContext)
	{
		if (InputDeviceMode == EBWInputDeviceMode::MouseKeyboard)
		{
			InputSubsystem->AddMappingContext(MouseKeyboardMappingContext, MouseKeyboardMappingPriority);
		}
		else
		{
			InputSubsystem->RemoveMappingContext(MouseKeyboardMappingContext);
		}
	}
}

void ABWPlayerController::ApplyInputDeviceMode()
{
	// Cursor visível só no modo teclado/mouse; no modo controle a mira é o personagem.
	bShowMouseCursor = (InputDeviceMode == EBWInputDeviceMode::MouseKeyboard);
}

void ABWPlayerController::HandleMove(const FInputActionValue& Value)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || !PlayerCameraManager)
	{
		return;
	}

	const FVector2D Input = Value.Get<FVector2D>();

	// Direção relativa à câmera (apenas o giro horizontal), para que "cima" no teclado
	// ou no analógico seja sempre "para frente na tela", qualquer que seja a câmera.
	const FRotator CameraYaw(0.0, PlayerCameraManager->GetCameraRotation().Yaw, 0.0);
	const FRotationMatrix YawMatrix(CameraYaw);
	const FVector Forward = YawMatrix.GetUnitAxis(EAxis::X);
	const FVector Right = YawMatrix.GetUnitAxis(EAxis::Y);

	// AddMovementInput passa pelo CharacterMovementComponent, que já faz predição
	// no cliente e correção pelo servidor. Nenhuma replicação manual é necessária.
	ControlledPawn->AddMovementInput(Forward, Input.Y);
	ControlledPawn->AddMovementInput(Right, Input.X);
}

void ABWPlayerController::HandleMoveStarted(const FInputActionValue& /*Value*/)
{
	bDirectMoveInputActive = true;

	// Movimento direto tem prioridade sobre o caminho do clique.
	if (UBWPathFollowComponent* PathFollow = GetBWPathFollowComponent())
	{
		PathFollow->StopPathFollowing();
	}
}

void ABWPlayerController::HandleMoveEnded(const FInputActionValue& /*Value*/)
{
	bDirectMoveInputActive = false;
}

void ABWPlayerController::HandleClickMoveStarted(const FInputActionValue& /*Value*/)
{
	if (!IsLocalController())
	{
		return;
	}

	// A1: clique só existe no modo teclado/mouse.
	if (InputDeviceMode != EBWInputDeviceMode::MouseKeyboard)
	{
		return;
	}

	// Enquanto o jogador dirige com WASD/stick, cliques são ignorados.
	if (bDirectMoveInputActive)
	{
		return;
	}

	bClickHeld = true;

	// Etapa 2.5: inimigo sob o cursor = atacar (procura por objetos do tipo Pawn,
	// que é o tipo da cápsula dos personagens).
	const ABWCharacterBase* SelfCharacter = Cast<ABWCharacterBase>(GetPawn());
	FHitResult PawnHit;
	const TArray<TEnumAsByte<EObjectTypeQuery>> PawnTypes = { UEngineTypes::ConvertToObjectType(ECC_Pawn) };
	if (SelfCharacter && GetHitResultUnderCursorForObjects(PawnTypes, false, PawnHit)
		&& SelfCharacter->IsHostileTo(PawnHit.GetActor()))
	{
		SetAttackTarget(PawnHit.GetActor());
		return;
	}

	// Clique no chão: abandona o alvo e anda.
	ClearAttackTarget();

	// B1: sem hit (céu/espaço vazio) não há destino; pedido ignorado.
	FHitResult Hit;
	if (!GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), false, Hit))
	{
		return;
	}

	RequestMoveTo(Hit.ImpactPoint);
}

void ABWPlayerController::HandleClickMoveCompleted(const FInputActionValue& /*Value*/)
{
	bClickHeld = false;
}

void ABWPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// PlayerTick só roda em controllers locais com jogador; o alvo do clique é estado local.
	UpdateAttackTarget(DeltaTime);
}

void ABWPlayerController::SetAttackTarget(AActor* NewTarget)
{
	if (AttackTarget.Get() != NewTarget)
	{
		ClearAttackTarget();
	}

	AttackTarget = NewTarget;
	bAttackIssued = false;
	AttackRepathTimer = 0.0f;

	UE_LOG(LogBWPlayerController, Log, TEXT("%s: alvo de ataque %s"), *GetNameSafe(this), *GetNameSafe(NewTarget));

	// Marcador: decal no chão, preso à cápsula do alvo (acompanha o movimento dele).
	if (!IsValid(TargetMarker) && TargetMarkerMaterial && NewTarget)
	{
		const ACharacter* TargetCharacter = Cast<ACharacter>(NewTarget);
		USceneComponent* AttachTo = TargetCharacter ? TargetCharacter->GetCapsuleComponent() : NewTarget->GetRootComponent();
		const float HalfHeight = TargetCharacter ? TargetCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 0.0f;

		// Decal projeta ao longo do X: pitch -90 aponta para baixo, até o chão.
		TargetMarker = UGameplayStatics::SpawnDecalAttached(TargetMarkerMaterial, TargetMarkerSize, AttachTo, NAME_None,
			FVector(0.0, 0.0, -HalfHeight), FRotator(-90.0, 0.0, 0.0), EAttachLocation::KeepRelativeOffset);
	}
	else if (!TargetMarkerMaterial)
	{
		UE_LOG(LogBWPlayerController, Warning, TEXT("%s: TargetMarkerMaterial não configurado no BP_BWPlayerController."), *GetNameSafe(this));
	}
}

void ABWPlayerController::ClearAttackTarget()
{
	AttackTarget.Reset();
	bAttackIssued = false;

	// O decal pode já ter sido destruído junto com o alvo (corpo removido).
	if (IsValid(TargetMarker))
	{
		TargetMarker->DestroyComponent();
	}
	TargetMarker = nullptr;
}

void ABWPlayerController::UpdateAttackTarget(float DeltaTime)
{
	AActor* Target = AttackTarget.Get();
	if (!Target)
	{
		return;
	}

	ABWCharacterBase* SelfCharacter = Cast<ABWCharacterBase>(GetPawn());
	if (!SelfCharacter || SelfCharacter->IsDead() || !SelfCharacter->IsHostileTo(Target))
	{
		// Morremos, o alvo morreu ou deixou de ser inimigo.
		ClearAttackTarget();
		return;
	}

	// Depois do primeiro golpe, soltar o botão encerra (o golpe em curso termina sozinho).
	if (bAttackIssued && !bClickHeld)
	{
		ClearAttackTarget();
		return;
	}

	const float Distance = FVector::Dist2D(SelfCharacter->GetActorLocation(), Target->GetActorLocation());
	if (Distance > MeleeEngageRange)
	{
		// Fora de alcance: anda até o alvo, recalculando o caminho porque ele se move.
		AttackRepathTimer -= DeltaTime;
		if (AttackRepathTimer <= 0.0f)
		{
			RequestMoveTo(Target->GetActorLocation());
			AttackRepathTimer = 0.25f;
		}
		return;
	}

	// No alcance: para, vira para o alvo e ataca (o ASC só reativa quando o golpe anterior terminou).
	if (UBWPathFollowComponent* PathFollow = GetBWPathFollowComponent())
	{
		PathFollow->StopPathFollowing();
	}

	FVector ToTarget = Target->GetActorLocation() - SelfCharacter->GetActorLocation();
	ToTarget.Z = 0.0;
	if (!ToTarget.IsNearlyZero())
	{
		SelfCharacter->SetActorRotation(ToTarget.Rotation());
	}

	if (ABWPlayerState* BWPlayerState = GetPlayerState<ABWPlayerState>())
	{
		if (UBWAbilitySystemComponent* AbilitySystem = BWPlayerState->GetBWAbilitySystemComponent())
		{
			// Primeiro golpe: mesmo caminho das teclas (avisa no log se o slot não tem habilidade).
			// Depois: "segurado", que só reativa habilidades de combo quando o golpe anterior acabou.
			if (!bAttackIssued)
			{
				AbilitySystem->BWAbilityInputTagPressed(InputTag_Attack_Primary);
			}
			else
			{
				AbilitySystem->BWAbilityInputTagHeld(InputTag_Attack_Primary);
			}
		}
	}
	bAttackIssued = true;
}

void ABWPlayerController::RequestMoveTo(const FVector& Destination)
{
	// Executa localmente (host ou cliente): o componente segue o caminho com
	// AddMovementInput e o CharacterMovement replica/valida no servidor.
	if (UBWPathFollowComponent* PathFollow = GetBWPathFollowComponent())
	{
		PathFollow->StartPathFollowing(Destination);
	}
}

UBWCameraComponent* ABWPlayerController::GetBWCameraComponent() const
{
	const APawn* ControlledPawn = GetPawn();
	return ControlledPawn ? ControlledPawn->FindComponentByClass<UBWCameraComponent>() : nullptr;
}

UBWPathFollowComponent* ABWPlayerController::GetBWPathFollowComponent() const
{
	const APawn* ControlledPawn = GetPawn();
	return ControlledPawn ? ControlledPawn->FindComponentByClass<UBWPathFollowComponent>() : nullptr;
}

void ABWPlayerController::HandleZoomWheel(const FInputActionValue& Value)
{
	if (UBWCameraComponent* BWCamera = GetBWCameraComponent())
	{
		BWCamera->AddZoomStep(Value.Get<float>());
	}
}

void ABWPlayerController::HandleZoomAxisChanged(const FInputActionValue& Value)
{
	if (UBWCameraComponent* BWCamera = GetBWCameraComponent())
	{
		BWCamera->SetZoomAxisInput(Value.Get<float>());
	}
}

void ABWPlayerController::HandleZoomAxisEnded(const FInputActionValue& /*Value*/)
{
	if (UBWCameraComponent* BWCamera = GetBWCameraComponent())
	{
		BWCamera->SetZoomAxisInput(0.0f);
	}
}

// ---------------------------------------------------------------------------------
// Etapa 1.4-b: roteamento botão → InputTag (infra das habilidades da 1.5).
// A MESMA Input Action está mapeada no teclado e no controle (IMC_DirectControl),
// então qualquer dispositivo dispara a MESMA tag — exigência da 1.5. A tag é
// levada no próprio bind (payload), padrão do Lyra (amostra oficial da Epic).
// ---------------------------------------------------------------------------------
void ABWPlayerController::BindTaggedInputActions(UEnhancedInputComponent* EnhancedInput)
{
	if (!BWInputConfig)
	{
		UE_LOG(LogBWPlayerController, Warning,
			TEXT("%s: BWInputConfig não configurado (DA_BWInputConfig). Os slots de habilidade não responderão."),
			*GetNameSafe(this));
		return;
	}

	for (const FBWInputAction& Entry : BWInputConfig->TaggedInputActions)
	{
		if (!Entry.InputAction || !Entry.InputTag.IsValid())
		{
			continue;
		}

		// Apertar = Started; soltar = Completed. A tag vai no payload do bind.
		EnhancedInput->BindAction(Entry.InputAction, ETriggerEvent::Started,
			this, &ThisClass::AbilityInputTagPressed, Entry.InputTag);
		EnhancedInput->BindAction(Entry.InputAction, ETriggerEvent::Completed,
			this, &ThisClass::AbilityInputTagReleased, Entry.InputTag);

		// Etapa 2.5: segurado (a cada quadro) — só habilidades de repetição respondem.
		EnhancedInput->BindAction(Entry.InputAction, ETriggerEvent::Triggered,
			this, &ThisClass::AbilityInputTagHeld, Entry.InputTag);
	}
}

void ABWPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	UE_LOG(LogBWPlayerController, Log,
		TEXT("%s: InputTag %s PRESSIONADA"),
		*GetNameSafe(this), *InputTag.ToString());

	// Etapa 1.5 (TECHNICAL_DESIGN §4): o controller SÓ repassa a tag ao ASC do
	// PlayerState — quem ativa é o ASC (servidor valida; cliente dono prediz).
	if (ABWPlayerState* BWPlayerState = GetPlayerState<ABWPlayerState>())
	{
		if (UBWAbilitySystemComponent* AbilitySystem = BWPlayerState->GetBWAbilitySystemComponent())
		{
			AbilitySystem->BWAbilityInputTagPressed(InputTag);
		}
	}
}

void ABWPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	UE_LOG(LogBWPlayerController, Log,
		TEXT("%s: InputTag %s solta"),
		*GetNameSafe(this), *InputTag.ToString());

	// Etapa 1.5: repassa ao ASC (decisão C1: habilidades de "segurar" virão depois).
	if (ABWPlayerState* BWPlayerState = GetPlayerState<ABWPlayerState>())
	{
		if (UBWAbilitySystemComponent* AbilitySystem = BWPlayerState->GetBWAbilitySystemComponent())
		{
			AbilitySystem->BWAbilityInputTagReleased(InputTag);
		}
	}
}

void ABWPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (ABWPlayerState* BWPlayerState = GetPlayerState<ABWPlayerState>())
	{
		if (UBWAbilitySystemComponent* AbilitySystem = BWPlayerState->GetBWAbilitySystemComponent())
		{
			AbilitySystem->BWAbilityInputTagHeld(InputTag);
		}
	}
}

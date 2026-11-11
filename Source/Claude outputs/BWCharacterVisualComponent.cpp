// Between Worlds — Etapa 2.1 (Visual por Data Asset)

#include "Character/BWCharacterVisualComponent.h"

#include "Character/BWCharacterVisualData.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/Character.h"

DEFINE_LOG_CATEGORY_STATIC(LogBWVisual, Log, All);

UBWCharacterVisualComponent::UBWCharacterVisualComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBWCharacterVisualComponent::OnRegister()
{
	Super::OnRegister();

	// OnRegister roda antes do BeginPlay, em servidor e clientes, e também no preview do editor.
	ApplyVisual();
}

#if WITH_EDITOR
void UBWCharacterVisualComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UBWCharacterVisualComponent, VisualData))
	{
		ApplyVisual();
	}
}
#endif

void UBWCharacterVisualComponent::ApplyVisual()
{
	if (!VisualData)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	USkeletalMeshComponent* MeshComp = Character ? Character->GetMesh() : nullptr;
	if (!MeshComp)
	{
		UE_LOG(LogBWVisual, Warning, TEXT("%s: o dono não é um ACharacter com malha; visual ignorado."), *GetNameSafe(GetOwner()));
		return;
	}

	// Carrega tudo antes de aplicar qualquer coisa: malha e AnimBP precisam do mesmo esqueleto.
	// Se a malha pedida não existir (pack removido), não aplicamos nada e o Blueprint
	// continua com o visual padrão dele, em vez de ficar com uma combinação quebrada.
	USkeletalMesh* NewMesh = nullptr;
	if (!VisualData->SkeletalMesh.IsNull())
	{
		NewMesh = VisualData->SkeletalMesh.LoadSynchronous();
		if (!NewMesh)
		{
			UE_LOG(LogBWVisual, Warning, TEXT("%s: malha %s não encontrada. Pack removido? Mantendo o visual padrão."),
				*VisualData->GetName(), *VisualData->SkeletalMesh.ToString());
			return;
		}
	}

	UClass* NewAnimClass = nullptr;
	if (!VisualData->AnimClass.IsNull())
	{
		NewAnimClass = VisualData->AnimClass.LoadSynchronous();
		if (!NewAnimClass)
		{
			UE_LOG(LogBWVisual, Warning, TEXT("%s: AnimBP %s não encontrado. Mantendo o visual padrão."),
				*VisualData->GetName(), *VisualData->AnimClass.ToString());
			return;
		}
	}

	if (NewMesh)
	{
		MeshComp->SetSkeletalMeshAsset(NewMesh);
	}

	if (NewAnimClass)
	{
		MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		MeshComp->SetAnimInstanceClass(NewAnimClass);
	}

	if (VisualData->bOverrideMeshTransform)
	{
		MeshComp->SetRelativeLocationAndRotation(VisualData->MeshRelativeLocation, VisualData->MeshRelativeRotation);
	}
}

UAnimMontage* UBWCharacterVisualComponent::GetMontage(FGameplayTag MontageTag) const
{
	return VisualData ? VisualData->LoadMontage(MontageTag) : nullptr;
}

UBWCharacterVisualComponent* UBWCharacterVisualComponent::FindVisualComponent(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UBWCharacterVisualComponent>() : nullptr;
}

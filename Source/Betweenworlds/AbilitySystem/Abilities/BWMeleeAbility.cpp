// Between Worlds — Etapa 2.4 (golpe corpo a corpo genérico)

#include "AbilitySystem/Abilities/BWMeleeAbility.h"

#include "AbilitySystem/Tags/BWCombatTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/BWCharacterBase.h"
#include "Character/BWCharacterVisualComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameplayEffect.h"

DEFINE_LOG_CATEGORY_STATIC(LogBWMelee, Log, All);

UBWMeleeAbility::UBWMeleeAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	HitEventTag = TAG_Event_Montage_Hit;
}

void UBWMeleeAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, /*bReplicateEndAbility=*/true, /*bWasCancelled=*/true);
		return;
	}

	HitActors.Reset();

	AActor* Avatar = GetAvatarActorFromActorInfo();
	const UBWCharacterVisualComponent* Visual = UBWCharacterVisualComponent::FindVisualComponent(Avatar);
	UAnimMontage* Montage = Visual ? Visual->GetMontage(MontageTag) : nullptr;
	if (!Montage)
	{
		UE_LOG(LogBWMelee, Warning, TEXT("%s: sem montage para %s em %s."),
			*GetName(), *MontageTag.ToString(), *GetNameSafe(Avatar));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Espera o quadro de acerto (pode haver mais de um notify no mesmo golpe).
	UAbilityTask_WaitGameplayEvent* WaitHit = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, HitEventTag, /*OptionalExternalTarget=*/nullptr, /*OnlyTriggerOnce=*/false, /*OnlyMatchExact=*/true);
	WaitHit->EventReceived.AddDynamic(this, &ThisClass::HandleHitEvent);
	WaitHit->ReadyForActivation();

	UAbilityTask_PlayMontageAndWait* PlayMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, Montage, PlayRate);
	PlayMontage->OnCompleted.AddDynamic(this, &ThisClass::HandleMontageFinished);
	PlayMontage->OnBlendOut.AddDynamic(this, &ThisClass::HandleMontageFinished);
	PlayMontage->OnInterrupted.AddDynamic(this, &ThisClass::HandleMontageCancelled);
	PlayMontage->OnCancelled.AddDynamic(this, &ThisClass::HandleMontageCancelled);
	PlayMontage->ReadyForActivation();
}

void UBWMeleeAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	HitActors.Reset();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBWMeleeAbility::HandleMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UBWMeleeAbility::HandleMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UBWMeleeAbility::HandleHitEvent(FGameplayEventData /*Payload*/)
{
	// Dano só existe no servidor (GAS_ARCHITECTURE §4); o cliente só vê a animação.
	if (HasAuthority(&CurrentActivationInfo))
	{
		ApplyHit();
	}
}

void UBWMeleeAbility::ApplyHit()
{
	ABWCharacterBase* Avatar = Cast<ABWCharacterBase>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
	if (!Avatar || !SourceASC || !World)
	{
		return;
	}

	if (!DamageEffect)
	{
		UE_LOG(LogBWMelee, Warning, TEXT("%s: DamageEffect não configurado."), *GetName());
		return;
	}

	const FVector Center = Avatar->GetActorLocation() + Avatar->GetActorForwardVector() * HitForwardOffset;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(BWMeleeHit), /*bTraceComplex=*/false, Avatar);
	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByChannel(Overlaps, Center, FQuat::Identity, ECC_Pawn, FCollisionShape::MakeSphere(HitRadius), Params);

	if (bDebugDrawHit)
	{
		DrawDebugSphere(World, Center, HitRadius, 16, FColor::Red, false, 1.5f);
	}

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		if (!Target || HitActors.Contains(Target) || !Avatar->IsHostileTo(Target))
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
		if (!TargetASC)
		{
			continue;
		}

		HitActors.Add(Target);

		FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(DamageEffect, GetAbilityLevel());
		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(TAG_Data_Damage, BaseDamage);
			SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
		}
	}
}

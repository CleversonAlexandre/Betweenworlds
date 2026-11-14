// Between Worlds — Etapa 2.7 (buff de grupo em área)

#include "AbilitySystem/Abilities/BWAreaBuffAbility.h"

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

DEFINE_LOG_CATEGORY_STATIC(LogBWAreaBuff, Log, All);

UBWAreaBuffAbility::UBWAreaBuffAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	TriggerEventTag = TAG_Event_Montage_Hit;
}

void UBWAreaBuffAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	bBuffApplied = false;

	AActor* Avatar = GetAvatarActorFromActorInfo();
	const UBWCharacterVisualComponent* Visual = UBWCharacterVisualComponent::FindVisualComponent(Avatar);
	UAnimMontage* Montage = (Visual && MontageTag.IsValid()) ? Visual->GetMontage(MontageTag) : nullptr;

	if (!Montage)
	{
		// Sem animação: aplica na hora e termina.
		if (HasAuthority(&CurrentActivationInfo))
		{
			ApplyBuff();
		}
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UAbilityTask_WaitGameplayEvent* WaitTrigger = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, TriggerEventTag, nullptr, /*OnlyTriggerOnce=*/true, /*OnlyMatchExact=*/true);
	WaitTrigger->EventReceived.AddDynamic(this, &ThisClass::HandleTriggerEvent);
	WaitTrigger->ReadyForActivation();

	UAbilityTask_PlayMontageAndWait* PlayMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, Montage);
	PlayMontage->OnCompleted.AddDynamic(this, &ThisClass::HandleMontageFinished);
	PlayMontage->OnBlendOut.AddDynamic(this, &ThisClass::HandleMontageFinished);
	PlayMontage->OnInterrupted.AddDynamic(this, &ThisClass::HandleMontageCancelled);
	PlayMontage->OnCancelled.AddDynamic(this, &ThisClass::HandleMontageCancelled);
	PlayMontage->ReadyForActivation();
}

void UBWAreaBuffAbility::HandleTriggerEvent(FGameplayEventData /*Payload*/)
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		ApplyBuff();
	}
}

void UBWAreaBuffAbility::HandleMontageFinished()
{
	// Montage sem notify: garante o buff no fim (servidor) para não "gastar" a recarga à toa.
	if (HasAuthority(&CurrentActivationInfo))
	{
		ApplyBuff();
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UBWAreaBuffAbility::HandleMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UBWAreaBuffAbility::ApplyBuff()
{
	if (bBuffApplied)
	{
		return;
	}
	bBuffApplied = true;

	ABWCharacterBase* Avatar = Cast<ABWCharacterBase>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
	if (!Avatar || !SourceASC || !World)
	{
		return;
	}

	int32 AlliesBuffed = 0;
	if (AllyEffect)
	{
		FCollisionQueryParams Params(SCENE_QUERY_STAT(BWAreaBuff), false);
		TArray<FOverlapResult> Overlaps;
		World->OverlapMultiByChannel(Overlaps, Avatar->GetActorLocation(), FQuat::Identity, ECC_Pawn,
			FCollisionShape::MakeSphere(Radius), Params);

		TSet<const AActor*> Seen;
		for (const FOverlapResult& Overlap : Overlaps)
		{
			const ABWCharacterBase* Ally = Cast<ABWCharacterBase>(Overlap.GetActor());
			if (!Ally || Seen.Contains(Ally) || Ally->GetTeamId() != Avatar->GetTeamId() || Ally->IsDead())
			{
				continue;
			}
			Seen.Add(Ally);

			UAbilitySystemComponent* AllyASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(const_cast<ABWCharacterBase*>(Ally));
			if (!AllyASC)
			{
				continue;
			}

			FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(AllyEffect, GetAbilityLevel());
			if (Spec.IsValid())
			{
				SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), AllyASC);
				++AlliesBuffed;
			}
		}
	}

	if (SelfEffect)
	{
		FGameplayEffectSpecHandle SelfSpec = MakeOutgoingGameplayEffectSpec(SelfEffect, GetAbilityLevel());
		if (SelfSpec.IsValid())
		{
			if (SelfSetByCallerTag.IsValid())
			{
				SelfSpec.Data->SetSetByCallerMagnitude(SelfSetByCallerTag, SelfSetByCallerMagnitude);
			}
			SourceASC->ApplyGameplayEffectSpecToSelf(*SelfSpec.Data.Get());
		}
	}

	if (CueTag.IsValid())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = Avatar->GetActorLocation();
		CueParams.Instigator = Avatar;
		CueParams.EffectCauser = Avatar;
		SourceASC->ExecuteGameplayCue(CueTag, CueParams);
	}

	if (bDebugDrawRadius)
	{
		DrawDebugSphere(World, Avatar->GetActorLocation(), Radius, 24, FColor::Yellow, false, 2.0f);
	}

	UE_LOG(LogBWAreaBuff, Log, TEXT("%s: %s aplicou buff em %d aliado(s)."),
		*GetName(), *GetNameSafe(Avatar), AlliesBuffed);
}

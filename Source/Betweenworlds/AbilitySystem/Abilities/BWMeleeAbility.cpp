// Between Worlds — Etapa 2.4 (golpe corpo a corpo genérico) / 2.5 (combo, mira suave, efeitos ao acertar)

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

	if (bFaceNearestTarget)
	{
		FaceNearestTarget();
	}

	const FGameplayTag TagToPlay = ResolveMontageTag();

	AActor* Avatar = GetAvatarActorFromActorInfo();
	const UBWCharacterVisualComponent* Visual = UBWCharacterVisualComponent::FindVisualComponent(Avatar);
	UAnimMontage* Montage = Visual ? Visual->GetMontage(TagToPlay) : nullptr;
	if (!Montage)
	{
		UE_LOG(LogBWMelee, Warning, TEXT("%s: sem montage para %s em %s."),
			*GetName(), *TagToPlay.ToString(), *GetNameSafe(Avatar));
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

	// Combo: golpe concluído avança; golpe cancelado recomeça do primeiro.
	if (ComboMontageTags.Num() > 0)
	{
		if (const UWorld* World = GetWorld())
		{
			LastComboEndTime = World->GetTimeSeconds();
		}
		ComboIndex = bWasCancelled ? 0 : (ComboIndex + 1) % ComboMontageTags.Num();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FGameplayTag UBWMeleeAbility::ResolveMontageTag()
{
	if (ComboMontageTags.Num() == 0)
	{
		return MontageTag;
	}

	const UWorld* World = GetWorld();
	const double Now = World ? World->GetTimeSeconds() : 0.0;
	if (Now - LastComboEndTime > ComboResetTime || !ComboMontageTags.IsValidIndex(ComboIndex))
	{
		ComboIndex = 0;
	}
	return ComboMontageTags[ComboIndex];
}

void UBWMeleeAbility::FaceNearestTarget() const
{
	ABWCharacterBase* Avatar = Cast<ABWCharacterBase>(GetAvatarActorFromActorInfo());
	UWorld* World = Avatar ? Avatar->GetWorld() : nullptr;
	if (!Avatar || !World || FaceTargetRange <= 0.0f)
	{
		return;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(BWMeleeFace), false, Avatar);
	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByChannel(Overlaps, Avatar->GetActorLocation(), FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(FaceTargetRange), Params);

	const FVector Forward = Avatar->GetActorForwardVector().GetSafeNormal2D();
	const float MinDot = FMath::Cos(FMath::DegreesToRadians(FaceTargetMaxAngle));

	const AActor* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	for (const FOverlapResult& Overlap : Overlaps)
	{
		const AActor* Candidate = Overlap.GetActor();
		if (!Candidate || !Avatar->IsHostileTo(Candidate))
		{
			continue;
		}

		const FVector ToCandidate = Candidate->GetActorLocation() - Avatar->GetActorLocation();
		if (FVector::DotProduct(Forward, ToCandidate.GetSafeNormal2D()) < MinDot)
		{
			continue;
		}

		const float DistSq = ToCandidate.SizeSquared2D();
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Candidate;
		}
	}

	if (Best)
	{
		FVector ToTarget = Best->GetActorLocation() - Avatar->GetActorLocation();
		ToTarget.Z = 0.0;
		Avatar->SetActorRotation(ToTarget.Rotation());
	}
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

	bool bHitSomething = false;
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
		bHitSomething = true;

		FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(DamageEffect, GetAbilityLevel());
		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(TAG_Data_Damage, BaseDamage);
			SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
		}
	}

	// Recompensa de quem golpeia (ex.: Fúria do Guerreiro), uma vez por acerto que atingiu alguém.
	if (bHitSomething)
	{
		for (const FBWMeleeSelfEffect& SelfEffect : SelfEffectsOnHit)
		{
			if (!SelfEffect.Effect)
			{
				continue;
			}

			FGameplayEffectSpecHandle SelfSpec = MakeOutgoingGameplayEffectSpec(SelfEffect.Effect, GetAbilityLevel());
			if (SelfSpec.IsValid())
			{
				if (SelfEffect.SetByCallerTag.IsValid())
				{
					SelfSpec.Data->SetSetByCallerMagnitude(SelfEffect.SetByCallerTag, SelfEffect.SetByCallerMagnitude);
				}
				SourceASC->ApplyGameplayEffectSpecToSelf(*SelfSpec.Data.Get());
			}
		}
	}
}

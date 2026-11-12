// Between Worlds — Etapa 2.3 (Morte e respawn)

#include "Character/BWDeathComponent.h"

#include "AbilitySystem/Tags/BWCombatTags.h"
#include "AbilitySystem/Tags/BWMontageTags.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Animation/AnimMontage.h"
#include "Character/BWCharacterVisualComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogBWDeath, Log, All);

UBWDeathComponent::UBWDeathComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UBWDeathComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UBWDeathComponent, DeathState);
}

void UBWDeathComponent::InitializeWithAbilitySystem(UAbilitySystemComponent* InAbilitySystem)
{
	if (AbilitySystem.Get() == InAbilitySystem)
	{
		SyncDeadTag();
		return;
	}

	UnbindFromAbilitySystem();
	AbilitySystem = InAbilitySystem;
	if (!InAbilitySystem)
	{
		return;
	}

	// Só o servidor decide a morte; clientes recebem o estado pelo OnRep.
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		DeathEventHandle = InAbilitySystem->GenericGameplayEventCallbacks.FindOrAdd(TAG_Event_Death)
			.AddUObject(this, &ThisClass::HandleDeathEvent);
	}

	// Corpo novo (respawn) vivo: limpa o State.Dead que o corpo anterior deixou no ASC.
	SyncDeadTag();
}

void UBWDeathComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindFromAbilitySystem();
	Super::EndPlay(EndPlayReason);
}

void UBWDeathComponent::UnbindFromAbilitySystem()
{
	if (UAbilitySystemComponent* ASC = AbilitySystem.Get())
	{
		if (DeathEventHandle.IsValid())
		{
			if (FGameplayEventMulticastDelegate* Delegate = ASC->GenericGameplayEventCallbacks.Find(TAG_Event_Death))
			{
				Delegate->Remove(DeathEventHandle);
			}
		}
	}
	DeathEventHandle.Reset();
	AbilitySystem.Reset();
}

void UBWDeathComponent::HandleDeathEvent(const FGameplayEventData* Payload)
{
	// O ASC do jogador sobrevive ao respawn; o evento só vale para o corpo atual (avatar).
	if (Payload && Payload->Target && Payload->Target != GetOwner())
	{
		return;
	}
	StartDeath();
}

void UBWDeathComponent::StartDeath()
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority() || DeathState == EBWDeathState::Dead)
	{
		return;
	}

	DeathState = EBWDeathState::Dead;
	UE_LOG(LogBWDeath, Log, TEXT("%s morreu."), *GetNameSafe(Owner));

	if (UAbilitySystemComponent* ASC = AbilitySystem.Get())
	{
		ASC->CancelAllAbilities();
	}

	// No servidor o OnRep não roda: aplica direto.
	ApplyDeathLocally();

	OnDeathStarted.Broadcast(Owner);
}

void UBWDeathComponent::OnRep_DeathState()
{
	if (DeathState == EBWDeathState::Dead)
	{
		ApplyDeathLocally();
	}
}

void UBWDeathComponent::SyncDeadTag()
{
	if (UAbilitySystemComponent* ASC = AbilitySystem.Get())
	{
		// Tag local (não replicada): cada máquina a deriva do DeathState replicado.
		ASC->SetLooseGameplayTagCount(TAG_State_Dead, DeathState == EBWDeathState::Dead ? 1 : 0);
	}
}

void UBWDeathComponent::ApplyDeathLocally()
{
	if (bDeathAppliedLocally)
	{
		return;
	}
	bDeathAppliedLocally = true;

	SyncDeadTag();

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	// O corpo não bloqueia ninguém nem dispara mais zonas/overlaps.
	if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Animação de morte pelo visual do personagem; sem montage, ragdoll.
	UAnimMontage* DeathMontage = nullptr;
	if (const UBWCharacterVisualComponent* Visual = UBWCharacterVisualComponent::FindVisualComponent(Character))
	{
		DeathMontage = Visual->GetMontage(TAG_Montage_Death);
	}

	if (DeathMontage && Character->PlayAnimMontage(DeathMontage) > 0.0f)
	{
		return;
	}

	if (USkeletalMeshComponent* Mesh = Character->GetMesh())
	{
		UE_LOG(LogBWDeath, Log, TEXT("%s: sem montage de morte, usando ragdoll."), *GetNameSafe(Character));
		Mesh->SetCollisionProfileName(TEXT("Ragdoll"));
		Mesh->SetSimulatePhysics(true);
	}
}

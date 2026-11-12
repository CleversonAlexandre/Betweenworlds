// Between Worlds — Etapa 2.4 (inimigo base)

#include "Character/BWEnemyCharacter.h"

#include "AbilitySystem/Attributes/BWHealthSet.h"
#include "AbilitySystem/BWAbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "AIController.h"
#include "Character/BWDeathComponent.h"
#include "GameplayEffect.h"

DEFINE_LOG_CATEGORY_STATIC(LogBWEnemy, Log, All);

ABWEnemyCharacter::ABWEnemyCharacter()
{
	TeamId = 1;

	AbilitySystemComponent = CreateDefaultSubobject<UBWAbilitySystemComponent>(TEXT("BWAbilitySystem"));
	AbilitySystemComponent->SetIsReplicated(true);
	// Minimal: ninguém controla o inimigo localmente, então efeitos completos não precisam
	// ir aos clientes; atributos (vida) replicam pelo próprio set.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	HealthSet = CreateDefaultSubobject<UBWHealthSet>(TEXT("HealthSet"));

	// Nasce já controlado pela IA, colocado no mapa ou spawnado.
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AAIController::StaticClass();
}

UAbilitySystemComponent* ABWEnemyCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ABWEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Owner e Avatar = este ator, em todas as máquinas.
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	if (DeathComponent)
	{
		DeathComponent->InitializeWithAbilitySystem(AbilitySystemComponent);
	}

	if (!HasAuthority())
	{
		return;
	}

	for (const TSubclassOf<UGameplayEffect>& EffectClass : DefaultEffects)
	{
		if (EffectClass)
		{
			FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
			Context.AddSourceObject(this);
			const FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.0f, Context);
			if (Spec.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}
	}

	if (AttackAbility)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AttackAbility, 1, INDEX_NONE, this));
	}
	else
	{
		UE_LOG(LogBWEnemy, Warning, TEXT("%s: AttackAbility não configurada."), *GetName());
	}

	if (DeathComponent)
	{
		DeathComponent->OnDeathStarted.AddUObject(this, &ThisClass::HandleDeathStarted);
	}
}

bool ABWEnemyCharacter::TryAttack()
{
	if (!HasAuthority() || !AttackAbility || IsDead() || IsAttacking())
	{
		return false;
	}
	return AbilitySystemComponent->TryActivateAbilityByClass(AttackAbility);
}

bool ABWEnemyCharacter::IsAttacking() const
{
	if (!AttackAbility || !AbilitySystemComponent)
	{
		return false;
	}
	const FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromClass(AttackAbility);
	return Spec && Spec->IsActive();
}

void ABWEnemyCharacter::HandleDeathStarted(AActor* /*DeadActor*/)
{
	// Para a IA (o StateTree deixa de rodar) e agenda a remoção do corpo.
	DetachFromControllerPendingDestroy();
	SetLifeSpan(CorpseLifetime);

	UE_LOG(LogBWEnemy, Log, TEXT("%s: corpo some em %.1f s."), *GetName(), CorpseLifetime);
}

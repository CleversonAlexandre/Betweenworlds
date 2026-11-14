// Between Worlds — Etapa 1.5 (GAS fundação) / 1.6 (atributos e Gameplay Effects) / 2.2 (sets por classe) / 2.3 (respawn) / 2.7 (CombatSet)

#include "Core/BWPlayerState.h"

#include "AbilitySystem/Attributes/BWHealthSet.h"
#include "AbilitySystem/Attributes/BWResourceSet.h"
#include "AbilitySystem/Attributes/BWCombatSet.h"
#include "AbilitySystem/BWAbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "AttributeSet.h"
#include "GameplayEffect.h"

DEFINE_LOG_CATEGORY_STATIC(LogBWPlayerState, Log, All);

ABWPlayerState::ABWPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UBWAbilitySystemComponent>(TEXT("BWAbilitySystem"));

	// Sem replicação do ASC não há lista de habilidades no cliente dono — e sem
	// lista, o input do cliente não ativa nada. MIXED: lista e efeitos ao dono;
	// efeitos leves nos simulados.
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// AttributeSets como subobjetos do MESMO ator dono do ASC: o ASC os encontra e
	// registra sozinho na inicialização. Replicam junto com este PlayerState.
	HealthSet = CreateDefaultSubobject<UBWHealthSet>(TEXT("HealthSet"));
	ResourceSet = CreateDefaultSubobject<UBWResourceSet>(TEXT("ResourceSet"));
	CombatSet = CreateDefaultSubobject<UBWCombatSet>(TEXT("CombatSet"));

	// GAS_ARCHITECTURE §2: efeitos, tags e atributos só chegam aos clientes na
	// frequência de rede DESTE ator. O padrão do PlayerState é baixo demais para gameplay.
	SetNetUpdateFrequency(100.0f);
}

UAbilitySystemComponent* ABWPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ABWPlayerState::GrantDefaultAbilities()
{
	// GiveAbility / efeitos iniciais são server-only; no cliente tudo chega replicado.
	if (!HasAuthority() || bAbilitiesGranted)
	{
		return;
	}
	bAbilitiesGranted = true;

	if (!AbilitySystemComponent)
	{
		return;
	}

	// 0) Sets da classe (ex.: Fúria). Precisam existir antes dos efeitos que os inicializam.
	//    AddSpawnedAttribute registra o set no ASC e o replica aos clientes.
	for (const TSubclassOf<UAttributeSet>& SetClass : ClassAttributeSets)
	{
		if (!SetClass || AbilitySystemComponent->GetAttributeSet(SetClass))
		{
			continue;
		}

		UAttributeSet* NewSet = NewObject<UAttributeSet>(this, SetClass);
		AbilitySystemComponent->AddSpawnedAttribute(NewSet);

		UE_LOG(LogBWPlayerState, Log, TEXT("%s: set de atributos %s criado"),
			*GetNameSafe(this), *SetClass->GetName());
	}

	// 1) Efeitos iniciais: as habilidades já nascem com recursos definidos.
	ApplyEffectsToSelf(DefaultEffects);

	// 2) Habilidades: a InputTag do slot vai nas tags dinâmicas do SPEC (não no asset).
	//    Sem InputTag = passiva (ativada por Gameplay Event, ex.: Fúria ao receber dano).
	for (const FBWAbilityGrant& Grant : DefaultAbilities)
	{
		if (!Grant.AbilityClass)
		{
			continue;
		}

		FGameplayAbilitySpec Spec(Grant.AbilityClass, /*Level=*/1, /*InputID=*/INDEX_NONE, /*SourceObject=*/this);
		if (Grant.InputTag.IsValid())
		{
			Spec.GetDynamicSpecSourceTags().AddTag(Grant.InputTag);
		}
		AbilitySystemComponent->GiveAbility(Spec);

		const FString SlotText = Grant.InputTag.IsValid()
			? FString::Printf(TEXT("no slot %s"), *Grant.InputTag.ToString())
			: FString(TEXT("como passiva"));

		UE_LOG(LogBWPlayerState, Log, TEXT("%s: habilidade %s concedida %s"),
			*GetNameSafe(this), *Grant.AbilityClass->GetName(), *SlotText);
	}
}

void ABWPlayerState::ApplyRespawnEffects()
{
	if (!HasAuthority())
	{
		return;
	}
	ApplyEffectsToSelf(RespawnEffects);
}

void ABWPlayerState::ApplyEffectsToSelf(const TArray<TSubclassOf<UGameplayEffect>>& Effects)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	for (const TSubclassOf<UGameplayEffect>& EffectClass : Effects)
	{
		if (!EffectClass)
		{
			continue;
		}

		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		Context.AddSourceObject(this);

		const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, /*Level=*/1.0f, Context);
		if (SpecHandle.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

			UE_LOG(LogBWPlayerState, Log, TEXT("%s: efeito %s aplicado"),
				*GetNameSafe(this), *EffectClass->GetName());
		}
	}
}

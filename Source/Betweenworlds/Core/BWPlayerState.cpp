// Between Worlds — Etapa 1.5 (GAS fundação) / 1.6 (atributos e Gameplay Effects)

#include "Core/BWPlayerState.h"

#include "AbilitySystem/Attributes/BWHealthSet.h"
#include "AbilitySystem/Attributes/BWResourceSet.h"
#include "AbilitySystem/BWAbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
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

	// 1) Efeitos iniciais primeiro: as habilidades já nascem com recursos definidos.
	for (const TSubclassOf<UGameplayEffect>& EffectClass : DefaultEffects)
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

			UE_LOG(LogBWPlayerState, Log, TEXT("%s: efeito inicial %s aplicado"),
				*GetNameSafe(this), *EffectClass->GetName());
		}
	}

	// 2) Habilidades: a InputTag do slot vai nas tags dinâmicas do SPEC (não no asset).
	for (const FBWAbilityGrant& Grant : DefaultAbilities)
	{
		if (!Grant.AbilityClass || !Grant.InputTag.IsValid())
		{
			continue;
		}

		FGameplayAbilitySpec Spec(Grant.AbilityClass, /*Level=*/1, /*InputID=*/INDEX_NONE, /*SourceObject=*/this);
		Spec.GetDynamicSpecSourceTags().AddTag(Grant.InputTag);
		AbilitySystemComponent->GiveAbility(Spec);

		UE_LOG(LogBWPlayerState, Log,
			TEXT("%s: habilidade %s concedida no slot %s"),
			*GetNameSafe(this),
			*Grant.AbilityClass->GetName(),
			*Grant.InputTag.ToString());
	}
}
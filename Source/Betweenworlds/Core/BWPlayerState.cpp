// Between Worlds — Etapa 1.5 (GAS fundação: ASC no PlayerState + primeira habilidade)
// Correção 1.5.1 (Claude): frequência de rede do PlayerState + API atual das tags do spec.

#include "Core/BWPlayerState.h"

#include "AbilitySystem/BWAbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"

DEFINE_LOG_CATEGORY_STATIC(LogBWPlayerState, Log, All);

ABWPlayerState::ABWPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UBWAbilitySystemComponent>(TEXT("BWAbilitySystem"));

	// Sem replicação do ASC não há lista de habilidades no cliente dono — e sem
	// lista, o input do cliente não ativa nada (padrão GASDocumentation). MIXED:
	// lista de habilidades ao cliente dono; efeitos mantidos leves nos simulados.
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// GAS_ARCHITECTURE §2: o ASC vive aqui, então efeitos, tags e atributos só
	// chegam aos clientes na frequência de rede DESTE ator. O padrão do PlayerState
	// é muito baixo para gameplay (pensado para placar); 100 é o valor usado com GAS
	// no PlayerState (Lyra/GASDocumentation).
	SetNetUpdateFrequency(100.0f);
}

UAbilitySystemComponent* ABWPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ABWPlayerState::GrantDefaultAbilities()
{
	// GiveAbility é server-only; no cliente a lista chega replicada (Mixed).
	if (!HasAuthority() || bAbilitiesGranted)
	{
		return;
	}
	bAbilitiesGranted = true;

	if (!AbilitySystemComponent)
	{
		return;
	}

	for (const FBWAbilityGrant& Grant : DefaultAbilities)
	{
		if (!Grant.AbilityClass || !Grant.InputTag.IsValid())
		{
			continue;
		}

		// A InputTag do slot vai nas tags dinâmicas do SPEC (não no asset): é por
		// ela que UBWAbilitySystemComponent::BWAbilityInputTagPressed encontra o
		// spec e ativa pelo handle. GetDynamicSpecSourceTags substitui o antigo
		// campo DynamicAbilityTags (descontinuado nas versões recentes da UE).
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
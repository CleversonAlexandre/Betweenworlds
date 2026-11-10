// Between Worlds — Etapa 1.5 (GAS fundação: ASC no PlayerState + primeira habilidade)
// Correção 1.5.1 (Claude): ativação pelo HANDLE do spec que tem a InputTag do slot.

#include "AbilitySystem/BWAbilitySystemComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogBWAbilitySystem, Log, All);

void UBWAbilitySystemComponent::BWAbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	// O slot (InputTag) fica nas tags dinâmicas do SPEC, gravadas no grant
	// (BWPlayerState::GrantDefaultAbilities). Por isso a busca é feita aqui, spec a
	// spec, e a ativação é pelo handle.
	//
	// NÃO usar TryActivateAbilitiesByTag: ela compara com as Asset Tags do próprio
	// asset da habilidade (ex.: GA_Dash), não com as tags do spec — com a tag de
	// slot só no spec, nada é encontrado e nada ativa (bug da 1.5 original).
	// Mantendo a tag fora do asset, a mesma habilidade pode ir para qualquer slot.
	bool bFoundAbilityForTag = false;
	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (!Spec.Ability || !Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			continue;
		}

		bFoundAbilityForTag = true;

		// Servidor: ativa com autoridade. Cliente dono + LocalPredicted: o GAS
		// prediz localmente e envia ServerTryActivateAbility sozinho.
		if (!Spec.IsActive())
		{
			TryActivateAbility(Spec.Handle);
		}
	}

	if (!bFoundAbilityForTag)
	{
		UE_LOG(LogBWAbilitySystem, Log,
			TEXT("Slot %s sem habilidade concedida (confira DefaultAbilities no BP_BWPlayerState)."),
			*InputTag.ToString());
	}
}

void UBWAbilitySystemComponent::BWAbilityInputTagReleased(const FGameplayTag& /*InputTag*/)
{
	// Decisão C1 da 1.5: sem habilidades de "segurar" por enquanto (o log de
	// "soltar" fica no BWPlayerController). Wire completo quando a primeira surgir.
}
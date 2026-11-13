// Between Worlds — Etapa 1.5 (GAS fundação: ASC no PlayerState + primeira habilidade)
// Correção 1.5.1 (Claude): ativação pelo HANDLE do spec que tem a InputTag do slot.
// Etapa 2.3: morto não ativa habilidades.
// Etapa 2.5: slot segurado reativa habilidades de combo.

#include "AbilitySystem/BWAbilitySystemComponent.h"

#include "AbilitySystem/Tags/BWCombatTags.h"
#include "AbilitySystem/Tags/BWGameplayTags.h"

DEFINE_LOG_CATEGORY_STATIC(LogBWAbilitySystem, Log, All);

void UBWAbilitySystemComponent::BWAbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	// Etapa 2.3: nenhum slot funciona com o personagem morto (vale para todas as
	// habilidades, sem configurar bloqueio em cada uma).
	if (HasMatchingGameplayTag(TAG_State_Dead))
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

void UBWAbilitySystemComponent::BWAbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid() || HasMatchingGameplayTag(TAG_State_Dead))
	{
		return;
	}

	// Chamado a cada quadro: sem log. Só habilidades de repetição, e só quando terminaram
	// (a próxima ativação é o próximo golpe do combo).
	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (!Spec.Ability || Spec.IsActive() || !Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			continue;
		}

		if (!Spec.Ability->GetAssetTags().HasTag(TAG_Ability_Input_RepeatWhileHeld))
		{
			continue;
		}

		TryActivateAbility(Spec.Handle);
	}
}

void UBWAbilitySystemComponent::BWAbilityInputTagReleased(const FGameplayTag& /*InputTag*/)
{
	// Decisão C1 da 1.5: sem habilidades de "segurar" por enquanto (o log de
	// "soltar" fica no BWPlayerController). Wire completo quando a primeira surgir.
}
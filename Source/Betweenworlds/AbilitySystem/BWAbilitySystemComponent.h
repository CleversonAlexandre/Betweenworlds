// Between Worlds — Etapa 1.5 (GAS fundação: ASC no PlayerState + primeira habilidade)

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h" // FGameplayTag — no .h: o UHT lê este arquivo isolado (lição do erro #3)
#include "BWAbilitySystemComponent.generated.h"

/**
 * AbilitySystemComponent do projeto (Etapa 1.5). Vive no BWPlayerState
 * (TECHNICAL_DESIGN §4: sobrevive a respawn e replica ao cliente dono).
 *
 * O BWPlayerCharacter liga este componente a si mesmo como AVATAR
 * (InitAbilityActorInfo: Owner = PlayerState, Avatar = personagem) e o
 * BWPlayerController repassa as tags de input (InputTag.Ability.1 / .2) para cá.
 */
UCLASS()
class BETWEENWORLDS_API UBWAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	/**
	 * Slot de habilidade pressionado (InputTag.Ability.1 / .2).
	 * Ativa a habilidade concedida com essa tag (o grant guarda a InputTag em
	 * DynamicAbilityTags). No servidor: ativa com autoridade. No cliente dono,
	 * com NetExecutionPolicy LocalPredicted: o GAS prediz e envia
	 * ServerTryActivateAbility sozinho.
	 */
	void BWAbilityInputTagPressed(const FGameplayTag& InputTag);

	/**
	 * Etapa 2.5: slot SEGURADO (chamado a cada quadro enquanto o botão está apertado).
	 * Só reativa habilidades marcadas com a Asset Tag Ability.Input.RepeatWhileHeld
	 * que não estejam ativas — é assim que segurar encadeia o combo do Corte.
	 */
	void BWAbilityInputTagHeld(const FGameplayTag& InputTag);

	/**
	 * Slot de habilidade solto. Decisão C1 da 1.5: nenhuma habilidade de "segurar"
	 * existe ainda — quando a primeira surgir, este método passa a repassar o
	 * evento via AbilitySpecInputPressed/AbilitySpecInputReleased (padrão Lyra).
	 */
	void BWAbilityInputTagReleased(const FGameplayTag& InputTag);
};

// Between Worlds — Etapa 1.2 (Fundação: rede) / 1.5 (GAS: interface com o ASC)

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h" // IAbilitySystemInterface (GAS) — Etapa 1.5
#include "GameFramework/Character.h"
#include "BWCharacterBase.generated.h"

class UAbilitySystemComponent;

/**
 * Base comum de jogadores e inimigos: colisão, movimento e (a partir da Etapa 1.5)
 * a interface com o Gameplay Ability System.
 *
 * Abstract: nunca é usado diretamente.
 */
UCLASS(Abstract)
class BETWEENWORLDS_API ABWCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABWCharacterBase();

	/**
	 * Etapa 1.5 (IAbilitySystemInterface): o ASC de cada personagem.
	 * Jogadores: o ASC do PlayerState (override no BWPlayerCharacter — TECHNICAL_DESIGN §4).
	 * Inimigos: terão o próprio ASC em etapa futura (padrão null até lá).
	 */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
};

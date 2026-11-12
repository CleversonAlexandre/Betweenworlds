// Between Worlds — Etapa 1.2 (Fundação: rede) / 1.5 (GAS: interface com o ASC) / 2.3 (morte) / 2.4 (times)

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h" // IAbilitySystemInterface (GAS) — Etapa 1.5
#include "GameFramework/Character.h"
#include "BWCharacterBase.generated.h"

class UAbilitySystemComponent;
class UBWDeathComponent;

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

	/** Etapa 2.3: morte deste corpo (jogadores e inimigos). */
	UBWDeathComponent* GetDeathComponent() const { return DeathComponent; }

	/** Etapa 2.4: time do personagem (0 = jogadores, 1 = inimigos). */
	uint8 GetTeamId() const { return TeamId; }

	/** Etapa 2.4: true se Other é um personagem de outro time e está vivo. Usado por golpes e IA. */
	bool IsHostileTo(const AActor* Other) const;

	/** Etapa 2.4: atalho para "este corpo está morto?". */
	bool IsDead() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BW|Death")
	TObjectPtr<UBWDeathComponent> DeathComponent;

	/** Personagens de times diferentes se atacam; do mesmo time, não. */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Team")
	uint8 TeamId = 0;
};

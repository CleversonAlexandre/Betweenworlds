// Between Worlds — Etapa 2.4 (inimigo base) / 2.7 (CombatSet)

#pragma once

#include "CoreMinimal.h"
#include "Character/BWCharacterBase.h"
#include "BWEnemyCharacter.generated.h"

class UBWAbilitySystemComponent;
class UBWHealthSet;
class UBWCombatSet;
class UGameplayAbility;
class UGameplayEffect;

/**
 * Inimigo controlado por IA (TECHNICAL_DESIGN §2 e §10).
 *
 * - ASC no próprio ator, replicação MINIMAL (atributos replicam; efeitos não precisam).
 * - Vida pelo mesmo BWHealthSet dos jogadores; morte pelo mesmo BWDeathComponent.
 * - A IA (StateTree no AIController) só decide QUANDO atacar; o ataque é uma habilidade
 *   do GAS (AttackAbility, normalmente filha de UBWMeleeAbility), ativada por TryAttack().
 * - Ao morrer, o corpo some depois de CorpseLifetime segundos.
 *
 * Abstract: o jogo usa um Blueprint filho (BP_BWEnemy_Minion) com visual e IA.
 */
UCLASS(Abstract)
class BETWEENWORLDS_API ABWEnemyCharacter : public ABWCharacterBase
{
	GENERATED_BODY()

public:
	ABWEnemyCharacter();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** Servidor: tenta ativar o ataque. false se morto, sem habilidade ou já atacando. */
	UFUNCTION(BlueprintCallable, Category = "BW|Enemy")
	bool TryAttack();

	/** true enquanto a habilidade de ataque está ativa. */
	UFUNCTION(BlueprintPure, Category = "BW|Enemy")
	bool IsAttacking() const;

	/** Distância em que a IA para de perseguir e ataca (cm). */
	UFUNCTION(BlueprintPure, Category = "BW|Enemy")
	float GetAttackRange() const { return AttackRange; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "BW|Abilities")
	TObjectPtr<UBWAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UBWHealthSet> HealthSet;

	UPROPERTY()
	TObjectPtr<UBWCombatSet> CombatSet;

	/** Efeitos aplicados ao nascer (servidor). Ex.: GE_Enemy_Init (vida inicial). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Abilities")
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

	/** Ataque principal (ex.: GA_Enemy_Melee). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Abilities")
	TSubclassOf<UGameplayAbility> AttackAbility;

	UPROPERTY(EditDefaultsOnly, Category = "BW|Enemy", meta = (ClampMin = "0.0"))
	float AttackRange = 150.0f;

	/** Segundos que o corpo fica no chão depois da morte. */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Enemy", meta = (ClampMin = "0.1"))
	float CorpseLifetime = 5.0f;

private:
	void HandleDeathStarted(AActor* DeadActor);
};

// Between Worlds — Etapa 2.4 (golpe corpo a corpo genérico)

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "BWMeleeAbility.generated.h"

class UGameplayEffect;

/**
 * Golpe corpo a corpo configurável por dados. Base de GA_Enemy_Melee (2.4) e do Corte (2.5).
 *
 *  1. Commit (custo/cooldown do Blueprint filho, se houver).
 *  2. Toca o montage da tag MontageTag, pedido ao visual do personagem (nada de asset direto).
 *  3. Espera o Gameplay Event HitEventTag (AnimNotify "BW Gameplay Event" no quadro do golpe).
 *  4. SÓ NO SERVIDOR: esfera à frente do personagem; cada inimigo vivo (outro time) atingido
 *     recebe DamageEffect com o SetByCaller Data.Damage = BaseDamage. Um acerto por alvo por golpe.
 *  5. Termina quando o montage acaba.
 */
UCLASS(Abstract)
class BETWEENWORLDS_API UBWMeleeAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UBWMeleeAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/** Montage a tocar (ex.: Montage.Attack.Primary.1), resolvido pelo BWCharacterVisualComponent. */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee", meta = (Categories = "Montage"))
	FGameplayTag MontageTag;

	/** Evento do quadro de acerto. */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee", meta = (Categories = "Event"))
	FGameplayTag HitEventTag;

	/** Efeito de dano (deve usar BWDamageExecution, que lê Data.Damage). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee")
	TSubclassOf<UGameplayEffect> DamageEffect;

	/** Dano base do golpe (Etapa 3: escalado por atributos dentro do BWDamageExecution). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee", meta = (ClampMin = "0.0"))
	float BaseDamage = 10.0f;

	/** Raio da esfera de acerto (cm). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee", meta = (ClampMin = "1.0"))
	float HitRadius = 120.0f;

	/** Distância do centro da esfera à frente do personagem (cm). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee")
	float HitForwardOffset = 100.0f;

	/** Velocidade do montage (futuro: velocidade de ataque). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee", meta = (ClampMin = "0.1"))
	float PlayRate = 1.0f;

	/** Desenha a esfera de acerto no servidor por alguns segundos (teste). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee|Debug")
	bool bDebugDrawHit = false;

private:
	UFUNCTION()
	void HandleMontageFinished();

	UFUNCTION()
	void HandleMontageCancelled();

	UFUNCTION()
	void HandleHitEvent(FGameplayEventData Payload);

	/** Servidor: aplica o golpe nos alvos dentro da esfera. */
	void ApplyHit();

	/** Alvos já atingidos nesta ativação (um acerto por alvo por golpe). */
	TSet<TWeakObjectPtr<AActor>> HitActors;
};

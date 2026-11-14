// Between Worlds — Etapa 2.4 (golpe corpo a corpo genérico) / 2.5 (combo, mira suave, efeitos ao acertar) / 2.6 (cone, cue de impacto)

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "BWMeleeAbility.generated.h"

class UGameplayEffect;

/** Forma da área de acerto. */
UENUM(BlueprintType)
enum class EBWMeleeHitShape : uint8
{
	/** Esfera à frente do personagem (golpes de alvo único/curtos). */
	Sphere,
	/** Cone a partir do personagem (golpes em área, ex.: Golpe Esmagador). */
	Cone
};

/** Efeito aplicado em quem golpeia quando o golpe acerta alguém (ex.: ganho de Fúria). */
USTRUCT(BlueprintType)
struct FBWMeleeSelfEffect
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee")
	TSubclassOf<UGameplayEffect> Effect;

	/** Opcional: SetByCaller a preencher (ex.: Data.Fury). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee", meta = (Categories = "Data"))
	FGameplayTag SetByCallerTag;

	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee")
	float SetByCallerMagnitude = 0.0f;
};

/**
 * Golpe corpo a corpo configurável por dados. Base de GA_Enemy_Melee (2.4) e do Corte (2.5).
 *
 *  1. Commit (custo/cooldown do Blueprint filho, se houver).
 *  2. Toca o montage da tag MontageTag, pedido ao visual do personagem (nada de asset direto).
 *  3. Espera o Gameplay Event HitEventTag (AnimNotify "BW Gameplay Event" no quadro do golpe).
 *  4. SÓ NO SERVIDOR: esfera à frente do personagem; cada inimigo vivo (outro time) atingido
 *     recebe DamageEffect com o SetByCaller Data.Damage = BaseDamage. Um acerto por alvo por golpe.
 *  5. Termina quando o montage acaba.
 *
 * Etapa 2.5:
 *  - Combo: com ComboMontageTags preenchido, cada ativação toca o próximo golpe se a anterior
 *    terminou há menos de ComboResetTime; senão volta ao primeiro. Segurar o botão reativa a
 *    habilidade (Asset Tag Ability.Input.RepeatWhileHeld), encadeando A → B → C → A…
 *  - Mira suave: ao ativar, vira para o inimigo vivo mais próximo à frente (mouse e controle).
 *  - SelfEffectsOnHit: aplicados em quem golpeia quando o golpe acerta (Fúria, em combate).
 *
 * Etapa 2.6:
 *  - HitShape = Cone: acerta todos os inimigos dentro de ConeRange e ConeHalfAngle.
 *  - ImpactCueTag: Gameplay Cue executado pelo servidor no ponto do impacto (todos veem).
 *  - Custo (ex.: Fúria) e recarga vêm dos campos Cost/Cooldown Gameplay Effect do GAS:
 *    sem recurso suficiente a habilidade não ativa.
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

	/**
	 * Combo: tags dos golpes em ordem (ex.: Montage.Attack.Primary.1, .2, .3).
	 * Vazio = golpe único com MontageTag.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee|Combo", meta = (Categories = "Montage"))
	TArray<FGameplayTag> ComboMontageTags;

	/** Segundos depois do fim de um golpe em que o próximo ainda continua o combo. */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee|Combo", meta = (ClampMin = "0.0"))
	float ComboResetTime = 0.6f;

	/** Ao ativar, vira para o inimigo vivo mais próximo à frente. */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee|Targeting")
	bool bFaceNearestTarget = true;

	/** Alcance da mira suave (cm). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee|Targeting", meta = (ClampMin = "0.0", EditCondition = "bFaceNearestTarget"))
	float FaceTargetRange = 350.0f;

	/** Ângulo máximo (graus) entre a frente do personagem e o alvo da mira suave. */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee|Targeting", meta = (ClampMin = "0.0", ClampMax = "180.0", EditCondition = "bFaceNearestTarget"))
	float FaceTargetMaxAngle = 90.0f;

	/** Efeitos em quem golpeia quando o golpe acerta pelo menos um alvo (ex.: GE_FuryGain + GE_InCombat). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee")
	TArray<FBWMeleeSelfEffect> SelfEffectsOnHit;

	/** Efeito de dano (deve usar BWDamageExecution, que lê Data.Damage). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee")
	TSubclassOf<UGameplayEffect> DamageEffect;

	/** Dano base do golpe (Etapa 3: escalado por atributos dentro do BWDamageExecution). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee", meta = (ClampMin = "0.0"))
	float BaseDamage = 10.0f;

	/** Forma da área de acerto. */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee")
	EBWMeleeHitShape HitShape = EBWMeleeHitShape::Sphere;

	/** Cone: alcance a partir do personagem (cm). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee", meta = (ClampMin = "1.0", EditCondition = "HitShape == EBWMeleeHitShape::Cone", EditConditionHides))
	float ConeRange = 250.0f;

	/** Cone: metade da abertura (graus). 45 = cone de 90°. */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee", meta = (ClampMin = "1.0", ClampMax = "180.0", EditCondition = "HitShape == EBWMeleeHitShape::Cone", EditConditionHides))
	float ConeHalfAngle = 45.0f;

	/** Gameplay Cue do impacto (ex.: GameplayCue.Warrior.CrushingBlow). Vazio = sem efeito visual. */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Melee", meta = (Categories = "GameplayCue"))
	FGameplayTag ImpactCueTag;

	/** Esfera: raio da esfera de acerto (cm). */
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

	/** Vira o avatar para o inimigo mais próximo à frente (servidor e cliente dono). */
	void FaceNearestTarget() const;

	/** Tag do montage desta ativação (combo ou golpe único). */
	FGameplayTag ResolveMontageTag();

	/** Combo: índice do próximo golpe e momento em que o último terminou. */
	int32 ComboIndex = 0;
	double LastComboEndTime = -1000.0;

	/** Alvos já atingidos nesta ativação (um acerto por alvo por golpe). */
	TSet<TWeakObjectPtr<AActor>> HitActors;
};

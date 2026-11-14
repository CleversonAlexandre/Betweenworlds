// Between Worlds — Etapa 2.7 (buff de grupo em área)

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "BWAreaBuffAbility.generated.h"

class UGameplayEffect;

/**
 * Habilidade de buff em área por dados (Grito de Guerra; futuras auras e gritos).
 *
 *  1. Commit (custo/recarga do Blueprint filho).
 *  2. Toca o montage da tag MontageTag (pelo visual do personagem).
 *  3. No evento TriggerEventTag (notify do montage) — ou na hora, se não houver montage —
 *     SÓ NO SERVIDOR aplica AllyEffect em todos os ALIADOS vivos dentro de Radius
 *     (mesmo time, incluindo quem usou), aplica SelfEffect em quem usou (ex.: Fúria)
 *     e executa CueTag no próprio personagem (todos veem).
 *  4. Termina quando o montage acaba.
 */
UCLASS(Abstract)
class BETWEENWORLDS_API UBWAreaBuffAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UBWAreaBuffAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "BW|AreaBuff", meta = (Categories = "Montage"))
	FGameplayTag MontageTag;

	/** Quadro em que o buff é aplicado (notify "BW Gameplay Event"). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|AreaBuff", meta = (Categories = "Event"))
	FGameplayTag TriggerEventTag;

	/** Efeito aplicado em cada aliado no raio (ex.: GE_Buff_WarCry, com duração). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|AreaBuff")
	TSubclassOf<UGameplayEffect> AllyEffect;

	/** Raio (cm) a partir de quem usou. */
	UPROPERTY(EditDefaultsOnly, Category = "BW|AreaBuff", meta = (ClampMin = "0.0"))
	float Radius = 800.0f;

	/** Efeito extra só em quem usou (ex.: GE_FuryGain). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|AreaBuff")
	TSubclassOf<UGameplayEffect> SelfEffect;

	/** SetByCaller opcional do SelfEffect (ex.: Data.Fury). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|AreaBuff", meta = (Categories = "Data"))
	FGameplayTag SelfSetByCallerTag;

	UPROPERTY(EditDefaultsOnly, Category = "BW|AreaBuff")
	float SelfSetByCallerMagnitude = 0.0f;

	/** Gameplay Cue executado no personagem ao aplicar (ex.: onda do grito). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|AreaBuff", meta = (Categories = "GameplayCue"))
	FGameplayTag CueTag;

	/** Desenha o raio no servidor (teste). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|AreaBuff|Debug")
	bool bDebugDrawRadius = false;

private:
	UFUNCTION()
	void HandleTriggerEvent(FGameplayEventData Payload);

	UFUNCTION()
	void HandleMontageFinished();

	UFUNCTION()
	void HandleMontageCancelled();

	/** Servidor: aplica o buff (uma vez por ativação). */
	void ApplyBuff();

	bool bBuffApplied = false;
};

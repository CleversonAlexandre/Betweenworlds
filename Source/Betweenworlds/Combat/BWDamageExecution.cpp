// Between Worlds — Etapa 2.2 (Fúria e dano) / 2.7 (multiplicador de dano)

#include "Combat/BWDamageExecution.h"

#include "AbilitySystem/Attributes/BWCombatSet.h"
#include "AbilitySystem/Attributes/BWHealthSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Tags/BWCombatTags.h"
#include "GameplayEffect.h"

DEFINE_LOG_CATEGORY_STATIC(LogBWDamage, Log, All);

namespace
{
	/** Atributos capturados pelo cálculo de dano. */
	struct FBWDamageCaptures
	{
		FGameplayEffectAttributeCaptureDefinition SourceDamageMultiplier;

		FBWDamageCaptures()
		{
			// Do atacante, no momento da execução (sem snapshot): vale o buff ativo agora.
			SourceDamageMultiplier = FGameplayEffectAttributeCaptureDefinition(
				UBWCombatSet::GetDamageMultiplierAttribute(), EGameplayEffectAttributeCaptureSource::Source, /*bSnapshot=*/false);
		}
	};

	const FBWDamageCaptures& DamageCaptures()
	{
		static FBWDamageCaptures Captures;
		return Captures;
	}
}

UBWDamageExecution::UBWDamageExecution()
{
	RelevantAttributesToCapture.Add(DamageCaptures().SourceDamageMultiplier);
}

void UBWDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// WarnIfNotFound = true: um efeito de dano sem "Data.Damage" é erro de configuração.
	const float BaseDamage = Spec.GetSetByCallerMagnitude(TAG_Data_Damage, /*WarnIfNotFound=*/true, /*DefaultIfNotFound=*/0.0f);

	// Multiplicador do atacante (só se ele tiver o CombatSet; senão, 1).
	float DamageMultiplier = 1.0f;
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const bool bSourceHasCombatSet = SourceASC && SourceASC->GetSet<UBWCombatSet>();
	bool bCaptured = false;
	if (bSourceHasCombatSet)
	{
		FAggregatorEvaluateParameters EvalParams;
		EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
		EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
		bCaptured = ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageCaptures().SourceDamageMultiplier, EvalParams, DamageMultiplier);
		if (!bCaptured)
		{
			DamageMultiplier = 1.0f;
		}
	}

	// Diagnóstico (Verbose: ligar com "log LogBWDamage Verbose" no console).
	UE_LOG(LogBWDamage, Verbose, TEXT("Dano: fonte %s | CombatSet %s | captura %s | base %.1f x mult %.2f = %.1f"),
		*GetNameSafe(SourceASC ? SourceASC->GetOwnerActor() : nullptr),
		bSourceHasCombatSet ? TEXT("sim") : TEXT("NAO"),
		bCaptured ? TEXT("ok") : TEXT("FALHOU"),
		BaseDamage, DamageMultiplier, BaseDamage * FMath::Max(DamageMultiplier, 0.0f));

	// Etapa 3: aplicar atributo principal, defesas, crítico e resistências aqui.
	const float FinalDamage = FMath::Max(BaseDamage * FMath::Max(DamageMultiplier, 0.0f), 0.0f);

	if (FinalDamage > 0.0f)
	{
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(UBWHealthSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, FinalDamage));
	}
	else
	{
		UE_LOG(LogBWDamage, Verbose, TEXT("%s: dano final 0, nada aplicado."), *GetNameSafe(Spec.Def));
	}
}

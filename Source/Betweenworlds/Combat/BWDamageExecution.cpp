// Between Worlds — Etapa 2.2 (Fúria e dano)

#include "Combat/BWDamageExecution.h"

#include "AbilitySystem/Attributes/BWHealthSet.h"
#include "AbilitySystem/Tags/BWCombatTags.h"
#include "GameplayEffect.h"

DEFINE_LOG_CATEGORY_STATIC(LogBWDamage, Log, All);

void UBWDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// WarnIfNotFound = true: um efeito de dano sem "Data.Damage" é erro de configuração.
	const float BaseDamage = Spec.GetSetByCallerMagnitude(TAG_Data_Damage, /*WarnIfNotFound=*/true, /*DefaultIfNotFound=*/0.0f);

	// Etapa 3: aplicar atributo principal, defesas, crítico e resistências aqui.
	const float FinalDamage = FMath::Max(BaseDamage, 0.0f);

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

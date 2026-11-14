// Between Worlds — Etapa 2.2 (Fúria e dano) / 2.7 (multiplicador de dano)

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "BWDamageExecution.generated.h"

/**
 * Cálculo de dano único do projeto (GAS_ARCHITECTURE §6). Roda só no servidor.
 *
 * Entrada: dano base no SetByCaller "Data.Damage" do efeito.
 * Saída:   o meta-atributo IncomingDamage do alvo (o BWHealthSet converte em perda de vida).
 *
 * Dano = base × DamageMultiplier do ATACANTE (BWCombatSet; buffs como o Grito de Guerra). Atributos (Força, defesas, crítico, resistências)
 * entram aqui na Etapa 3, quando as fórmulas forem aprovadas, sem mudar quem aplica dano.
 */
UCLASS()
class BETWEENWORLDS_API UBWDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UBWDamageExecution();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};

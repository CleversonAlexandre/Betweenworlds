// Between Worlds — Etapa 2.4 (IA: tarefas de StateTree)

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "BWStateTreeTasks.generated.h"

/*
 * Tarefas de StateTree dos inimigos (TECHNICAL_DESIGN §10).
 * A lógica de verdade fica em C++ reutilizável (ABWEnemyCharacter::TryAttack, busca de alvo);
 * as tarefas só ligam essa lógica ao StateTree. Um Behavior Tree poderia chamar as mesmas
 * funções sem mudar o inimigo.
 *
 * "Context" = ligado automaticamente ao ator do StateTree (o inimigo).
 */

// ---------------------------------------------------------------------------------
// BW Find Nearest Player: mantém em Target o jogador vivo mais próximo.
// Fica Running enquanto existir alvo (para estados filhos usarem o Target);
// falha quando não há jogador vivo dentro de MaxDistance.
// ---------------------------------------------------------------------------------

USTRUCT()
struct FBWFindNearestPlayerTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor = nullptr;

	/** Jogadores além desta distância são ignorados (cm). */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float MaxDistance = 3000.0f;

	/** Jogador alvo (ligar ao "Target Actor" do Move To nos estados filhos). */
	UPROPERTY(EditAnywhere, Category = "Output")
	TObjectPtr<AActor> Target = nullptr;
};

USTRUCT(meta = (DisplayName = "BW Find Nearest Player", Category = "BW"))
struct BETWEENWORLDS_API FBWStateTreeFindNearestPlayerTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBWFindNearestPlayerTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

	/** Jogador vivo e hostil mais próximo de Self dentro de MaxDistance (ou nullptr). */
	static AActor* FindNearestPlayer(const AActor* Self, float MaxDistance);
};

// ---------------------------------------------------------------------------------
// BW Enemy Attack: ativa o ataque do inimigo e fica Running até a habilidade terminar.
// ---------------------------------------------------------------------------------

USTRUCT()
struct FBWEnemyAttackTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor = nullptr;

	/** Opcional: vira o inimigo para este ator antes de atacar. */
	UPROPERTY(EditAnywhere, Category = "Input", meta = (Optional))
	TObjectPtr<AActor> FaceTarget = nullptr;
};

USTRUCT(meta = (DisplayName = "BW Enemy Attack", Category = "BW"))
struct BETWEENWORLDS_API FBWStateTreeEnemyAttackTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBWEnemyAttackTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};

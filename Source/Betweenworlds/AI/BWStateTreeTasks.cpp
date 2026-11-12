// Between Worlds — Etapa 2.4 (IA: tarefas de StateTree)

#include "AI/BWStateTreeTasks.h"

#include "Character/BWEnemyCharacter.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "StateTreeExecutionContext.h"

// ----------------------------------------------------------------------------- Find Nearest Player

AActor* FBWStateTreeFindNearestPlayerTask::FindNearestPlayer(const AActor* Self, float MaxDistance)
{
	const ABWCharacterBase* SelfCharacter = Cast<ABWCharacterBase>(Self);
	const UWorld* World = Self ? Self->GetWorld() : nullptr;
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	if (!SelfCharacter || !GameState)
	{
		return nullptr;
	}

	AActor* Best = nullptr;
	float BestDistSq = FMath::Square(MaxDistance);

	// Lista de jogadores da sessão (igual em listen e dedicado); o pawn de cada um é o alvo.
	for (const APlayerState* PlayerState : GameState->PlayerArray)
	{
		APawn* Pawn = PlayerState ? PlayerState->GetPawn() : nullptr;
		if (!Pawn || !SelfCharacter->IsHostileTo(Pawn))
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(Self->GetActorLocation(), Pawn->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Pawn;
		}
	}
	return Best;
}

EStateTreeRunStatus FBWStateTreeFindNearestPlayerTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& /*Transition*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	Data.Target = FindNearestPlayer(Data.Actor, Data.MaxDistance);
	return Data.Target ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FBWStateTreeFindNearestPlayerTask::Tick(FStateTreeExecutionContext& Context, const float /*DeltaTime*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	Data.Target = FindNearestPlayer(Data.Actor, Data.MaxDistance);
	return Data.Target ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
}

// ----------------------------------------------------------------------------- Enemy Attack

EStateTreeRunStatus FBWStateTreeEnemyAttackTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& /*Transition*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	ABWEnemyCharacter* Enemy = Cast<ABWEnemyCharacter>(Data.Actor);
	if (!Enemy)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (Data.FaceTarget)
	{
		FVector ToTarget = Data.FaceTarget->GetActorLocation() - Enemy->GetActorLocation();
		ToTarget.Z = 0.0;
		if (!ToTarget.IsNearlyZero())
		{
			Enemy->SetActorRotation(ToTarget.Rotation());
		}
	}

	return Enemy->TryAttack() ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FBWStateTreeEnemyAttackTask::Tick(FStateTreeExecutionContext& Context, const float /*DeltaTime*/) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	const ABWEnemyCharacter* Enemy = Cast<ABWEnemyCharacter>(Data.Actor);
	if (!Enemy || Enemy->IsDead())
	{
		return EStateTreeRunStatus::Failed;
	}
	return Enemy->IsAttacking() ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
}

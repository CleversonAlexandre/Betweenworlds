// Between Worlds — Etapa 1.2 (Fundação: rede) / 1.5 (GAS: PlayerState padrão) / 2.3 (respawn)

#include "Core/BWGameMode.h"

#include "Core/BWPlayerController.h"
#include "Core/BWPlayerState.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogBWGameMode, Log, All);

ABWGameMode::ABWGameMode()
{
	// Valor padrão de segurança. O BP_BWGameMode substitui pelo BP_BWPlayerController,
	// que é onde os assets de input são configurados.
	PlayerControllerClass = ABWPlayerController::StaticClass();

	// Etapa 1.5: PlayerState padrão com o AbilitySystemComponent do GAS
	// (TECHNICAL_DESIGN §4). O BP_BWGameMode substitui pelo BP_BWPlayerState,
	// que é onde as habilidades concedidas (DefaultAbilities) são configuradas.
	PlayerStateClass = ABWPlayerState::StaticClass();
}

void ABWGameMode::RequestRespawn(AController* Controller)
{
	if (!Controller)
	{
		return;
	}

	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle,
		FTimerDelegate::CreateUObject(this, &ThisClass::RespawnNow, TWeakObjectPtr<AController>(Controller)),
		FMath::Max(RespawnDelay, 0.01f), /*bLoop=*/false);

	UE_LOG(LogBWGameMode, Log, TEXT("%s: respawn em %.1f s."), *GetNameSafe(Controller), RespawnDelay);
}

void ABWGameMode::RespawnNow(TWeakObjectPtr<AController> WeakController)
{
	AController* Controller = WeakController.Get();
	if (!Controller)
	{
		return; // Jogador saiu durante a espera.
	}

	if (APawn* OldPawn = Controller->GetPawn())
	{
		Controller->UnPossess();
		OldPawn->Destroy();
	}

	// Escolhe um PlayerStart e cria o personagem novo. O PossessedBy dele religa o ASC
	// do PlayerState (que sobreviveu) ao corpo novo.
	RestartPlayer(Controller);

	// Vida, recursos etc. voltam ao valor inicial (efeitos periódicos continuam ativos).
	if (ABWPlayerState* BWPlayerState = Controller->GetPlayerState<ABWPlayerState>())
	{
		BWPlayerState->ApplyRespawnEffects();
	}

	UE_LOG(LogBWGameMode, Log, TEXT("%s: respawn concluído."), *GetNameSafe(Controller));
}

// Between Worlds — Etapa 1.2 (Fundação: rede) / 1.5 (GAS: PlayerState padrão)

#include "Core/BWGameMode.h"

#include "Core/BWPlayerController.h"
#include "Core/BWPlayerState.h"

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

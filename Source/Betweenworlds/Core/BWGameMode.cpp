// Between Worlds — Etapa 1.2 (Fundação: rede)

#include "Core/BWGameMode.h"

#include "Core/BWPlayerController.h"

ABWGameMode::ABWGameMode()
{
	// Valor padrão de segurança. O BP_BWGameMode substitui pelo BP_BWPlayerController,
	// que é onde os assets de input são configurados.
	PlayerControllerClass = ABWPlayerController::StaticClass();
}

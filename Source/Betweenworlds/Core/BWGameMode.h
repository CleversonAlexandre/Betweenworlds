// Between Worlds — Etapa 1.2 (Fundação: rede) / 1.5 (GAS: PlayerState padrão)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BWGameMode.generated.h"

/**
 * Regras da sessão. Existe SOMENTE no servidor (host do listen server ou servidor dedicado).
 *
 * Herda de AGameModeBase (e não de AGameMode) porque o jogo não usa o fluxo de
 * "partida" (warmup/em andamento/fim) que o AGameMode traz para jogos competitivos.
 *
 * Nesta etapa só define as classes padrão. A escolha de PlayerStart usa o
 * comportamento padrão da engine, que já prioriza pontos de spawn desocupados.
 *
 * Etapa 1.5: define o BWPlayerState como padrão (PlayerStateClass) — o
 * PlayerState carrega o AbilitySystemComponent do GAS. O BP_BWGameMode aponta
 * para o BP_BWPlayerState, onde ficam as habilidades concedidas.
 *
 * Abstract: o jogo usa o filho em Blueprint (BP_BWGameMode), onde ficam
 * as referências a assets (personagem, controller configurado).
 */
UCLASS(Abstract)
class BETWEENWORLDS_API ABWGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABWGameMode();
};

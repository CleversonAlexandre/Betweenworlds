// Between Worlds — Etapa 1.2 (Fundação: rede) / 1.5 (GAS: PlayerState padrão) / 2.3 (respawn)

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

	/**
	 * Etapa 2.3: agenda o respawn do jogador deste controller depois de RespawnDelay.
	 * O corpo antigo fica no chão até lá; então é destruído e um novo nasce num
	 * PlayerStart, com os RespawnEffects do PlayerState aplicados.
	 */
	void RequestRespawn(AController* Controller);

protected:
	/** Segundos entre a morte e o respawn. */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Respawn", meta = (ClampMin = "0.0"))
	float RespawnDelay = 5.0f;

private:
	void RespawnNow(TWeakObjectPtr<AController> WeakController);
};

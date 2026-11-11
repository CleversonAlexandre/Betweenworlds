// Between Worlds — Etapa 1.7 (HUD: painel de status)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BWHUD.generated.h"

class UBWStatusPanelWidget;

/**
 * HUD do jogador. A classe AHUD só existe no controller LOCAL: o servidor dedicado
 * nunca cria HUD nem widgets, e no listen server só o host tem a sua.
 *
 * Cria o painel de status e o liga ao ASC assim que o PlayerState (dono do ASC)
 * estiver disponível. No cliente o PlayerState chega pela rede, às vezes depois da
 * HUD existir; por isso a ligação é tentada por um timer curto que se desliga
 * sozinho ao conseguir (sem Tick, sem custo depois disso).
 *
 * Abstract: o jogo usa BP_BWHUD, onde se escolhe o WBP do painel.
 */
UCLASS(Abstract)
class BETWEENWORLDS_API ABWHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Widget do painel de status (WBP_StatusPanel, filho de BWStatusPanelWidget). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|HUD")
	TSubclassOf<UBWStatusPanelWidget> StatusPanelClass;

	/** Intervalo entre tentativas de ligar o painel ao ASC (segundos). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|HUD", meta = (ClampMin = "0.05"))
	float BindRetryInterval = 0.2f;

private:
	/** Tenta ligar o painel ao ASC do PlayerState local; para o timer quando consegue. */
	void TryBindStatusPanel();

	UPROPERTY(Transient)
	TObjectPtr<UBWStatusPanelWidget> StatusPanel;

	FTimerHandle BindTimerHandle;

	/** True depois que o painel foi ligado ao ASC. */
	bool bStatusPanelBound = false;
};

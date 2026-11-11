// Between Worlds — Etapa 1.7 (HUD: painel de status)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BWStatusPanelWidget.generated.h"

class UAbilitySystemComponent;
struct FOnAttributeChangeData;

/**
 * Painel de status (GDD §12): vida, mana e stamina do jogador LOCAL.
 *
 * Contrato GAS → UI (GAS_ARCHITECTURE §10): a UI só ESCUTA os delegates de mudança
 * de atributo do ASC. Nunca altera gameplay e não usa Tick nem consulta valores a
 * cada frame. Cada mudança chega ao Blueprint (WBP_StatusPanel) por eventos, e o
 * Blueprint cuida só do visual.
 *
 * Inicializado pelo ABWHUD quando o PlayerState (dono do ASC) estiver disponível.
 */
UCLASS(Abstract)
class BETWEENWORLDS_API UBWStatusPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Liga o painel ao ASC do jogador e envia os valores atuais imediatamente. */
	void InitializeWithAbilitySystem(UAbilitySystemComponent* InAbilitySystem);

protected:
	virtual void NativeDestruct() override;

	/** Vida mudou (atual ou máximo). Implementar no WBP: barra vermelha + texto "atual / máx". */
	UFUNCTION(BlueprintImplementableEvent, Category = "BW|HUD")
	void OnHealthChanged(float Current, float Max);

	/** Mana mudou. Implementar no WBP: barra azul. */
	UFUNCTION(BlueprintImplementableEvent, Category = "BW|HUD")
	void OnManaChanged(float Current, float Max);

	/** Stamina mudou. Implementar no WBP: barra verde. */
	UFUNCTION(BlueprintImplementableEvent, Category = "BW|HUD")
	void OnStaminaChanged(float Current, float Max);

private:
	void HandleAttributeChanged(const FOnAttributeChangeData& Data);
	void BroadcastAll();
	void UnbindFromAbilitySystem();

	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystem;
};

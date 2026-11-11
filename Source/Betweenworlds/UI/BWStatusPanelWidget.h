// Between Worlds — Etapa 1.7 (HUD: painel de status) / 2.2 (recurso da classe)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BWStatusPanelWidget.generated.h"

class UAbilitySystemComponent;
struct FOnAttributeChangeData;

/** Qual recurso a segunda barra do painel mostra (GDD §12). */
UENUM(BlueprintType)
enum class EBWClassResource : uint8
{
	Mana UMETA(DisplayName = "Mana"),
	Fury UMETA(DisplayName = "Fúria")
};

/**
 * Painel de status (GDD §12): vida, recurso da classe e stamina do jogador LOCAL.
 *
 * Contrato GAS → UI (GAS_ARCHITECTURE §10): a UI só ESCUTA os delegates de mudança
 * de atributo do ASC. Nunca altera gameplay e não usa Tick nem consulta valores a
 * cada frame. Cada mudança chega ao Blueprint (WBP_StatusPanel) por eventos, e o
 * Blueprint cuida só do visual.
 *
 * Recurso da classe: se o ASC tiver o set de Fúria, a segunda barra mostra Fúria;
 * senão, Mana. Quando existir uma definição de classe, ela passa a decidir isso.
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

	/**
	 * Recurso da classe mudou. Implementar no WBP: segunda barra, com a cor escolhida
	 * por Resource (Mana = azul, Fúria = laranja).
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "BW|HUD")
	void OnClassResourceChanged(float Current, float Max, EBWClassResource Resource);

	/** Stamina mudou. Implementar no WBP: barra verde. */
	UFUNCTION(BlueprintImplementableEvent, Category = "BW|HUD")
	void OnStaminaChanged(float Current, float Max);

private:
	void HandleAttributeChanged(const FOnAttributeChangeData& Data);
	void BroadcastAll();
	void UnbindFromAbilitySystem();

	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystem;
};

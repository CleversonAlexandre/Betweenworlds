// Between Worlds — Etapa 1.2 (Fundação: rede)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BWPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/**
 * Controller do jogador. Existe no servidor e no cliente dono; input só roda no cliente local.
 *
 * Etapa 1.2: apenas movimento direto (WASD / analógico), relativo à câmera.
 * Etapa 1.4 adiciona clique para mover, troca automática de dispositivo
 * e o roteamento de InputTags para o GAS.
 */
UCLASS()
class BETWEENWORLDS_API ABWPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	/** Converte o eixo 2D de input em movimento no plano, relativo à direção da câmera. */
	void HandleMove(const FInputActionValue& Value);

	/** Mapeamento para WASD e controle (IMC_DirectControl). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Input")
	TObjectPtr<UInputMappingContext> DirectControlMappingContext;

	/** Prioridade do mapeamento acima. Maior vence quando dois contextos usam a mesma tecla. */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Input")
	int32 DirectControlMappingPriority = 0;

	/** Ação de movimento, tipo Axis2D (IA_Move). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Input")
	TObjectPtr<UInputAction> MoveAction;
};

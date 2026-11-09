// Between Worlds — Etapa 1.2 (Fundação: rede) / 1.3 (câmera)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BWPlayerController.generated.h"

class UBWCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/**
 * Controller do jogador. Existe no servidor e no cliente dono; input só roda no cliente local.
 *
 * Etapa 1.2: movimento direto (WASD / analógico), relativo à câmera.
 * Etapa 1.3: zoom da câmera (roda do mouse e analógico), repassado ao
 * BWCameraComponent do pawn possuído. Como todo input, nunca chega ao servidor.
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

	/** Zoom discreto (roda do mouse): um passo por clique. */
	void HandleZoomWheel(const FInputActionValue& Value);

	/** Zoom contínuo (analógico): alimenta o componente enquanto o stick está inclinado. */
	void HandleZoomAxisChanged(const FInputActionValue& Value);

	/** Zera a entrada analógica quando o stick é solto. */
	void HandleZoomAxisEnded(const FInputActionValue& /*Value*/);

	/** Componente de câmera do pawn possuído (ou null). */
	UBWCameraComponent* GetBWCameraComponent() const;

	/** Mapeamento para WASD e controle (IMC_DirectControl). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Input")
	TObjectPtr<UInputMappingContext> DirectControlMappingContext;

	/** Prioridade do mapeamento acima. Maior vence quando dois contextos usam a mesma tecla. */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Input")
	int32 DirectControlMappingPriority = 0;

	/** Ação de movimento, tipo Axis2D (IA_Move). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Input")
	TObjectPtr<UInputAction> MoveAction;

	/** Ação de zoom da roda do mouse, tipo Axis1D (IA_ZoomWheel). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Input")
	TObjectPtr<UInputAction> ZoomWheelAction;

	/** Ação de zoom do analógico, tipo Axis1D (IA_ZoomAxis). */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Input")
	TObjectPtr<UInputAction> ZoomAxisAction;
};

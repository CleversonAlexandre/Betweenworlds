// Between Worlds — Etapa 1.3 (Fundação: câmera)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/EngineTypes.h"
#include "BWCameraSettings.generated.h"

class UMaterialInterface;

/**
 * Todos os valores da câmera isométrica em um único Data Asset.
 *
 * Regra do projeto (handoff §11.10): nenhum valor de gameplay hardcoded.
 * Mudar aqui muda o comportamento sem recompilar.
 *
 * O diretor cria o DA_BWCameraSettings no editor a partir desta classe
 * e o atribui ao componente BWCamera no BP_BWPlayerCharacter.
 */
UCLASS(BlueprintType)
class BETWEENWORLDS_API UBWCameraSettings : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ----- Enquadramento -----

	/** Inclinação da câmera em graus (negativa = vista de cima). Faixa aprovada: -50 a -60. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BW|Enquadramento")
	float Pitch = -55.0f;

	/** Comprimento do braço no início do jogo (distância personagem -> câmera). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BW|Enquadramento")
	float DefaultArmLength = 1400.0f;

	/** Deslocamento da câmera em relação ao personagem, para ajuste fino de enquadramento. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BW|Enquadramento")
	FVector SocketOffset = FVector::ZeroVector;

	// ----- Zoom -----

	/** Zoom mais aproximado (limite inferior do braço). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BW|Zoom")
	float MinArmLength = 700.0f;

	/** Zoom mais afastado (limite superior do braço). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BW|Zoom")
	float MaxArmLength = 2200.0f;

	/** Passo de zoom por clique da roda do mouse (unidades do braço). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BW|Zoom")
	float ZoomStepWheel = 120.0f;

	/** Velocidade do zoom contínuo do analógico (unidades por segundo, stick no máximo). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BW|Zoom")
	float ZoomRateGamepad = 900.0f;

	/** Suavidade da interpolação do zoom (FInterpTo). Maior = responde mais rápido. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BW|Zoom")
	float ZoomSmoothness = 8.0f;

	// ----- Fade de obstruções -----

	/** Intervalo entre traces de oclusão, em segundos. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BW|Fade")
	float FadeTraceInterval = 0.1f;

	/** Opacidade do obstrutor: 0 = invisível, 1 = opaco. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BW|Fade", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FadeOpacity = 0.15f;

	/** Velocidade da transição do fade (FInterpTo). Maior = esmaece/aparece mais rápido. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BW|Fade")
	float FadeInterpSpeed = 10.0f;

	/**
	 * Canal de colisão do trace de oclusão.
	 * ECC_Camera (decisão C1): canal dedicado; um mesh indevido bloqueando o
	 * fade é resolvido nas respostas de colisão dele, sem tocar no código.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BW|Fade")
	TEnumAsByte<ECollisionChannel> FadeTraceChannel = ECC_Camera;

	/**
	 * Material base do fade. DEVE ser Translucent + Unlit, com o scalar
	 * parameter "FadeAmount" ligado no Opacity (ver PASSOS_NO_EDITOR).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BW|Fade")
	TObjectPtr<UMaterialInterface> FadeMaterial = nullptr;
};

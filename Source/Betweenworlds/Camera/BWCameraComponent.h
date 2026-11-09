// Between Worlds — Etapa 1.3 (Fundação: câmera)

#pragma once

#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "BWCameraComponent.generated.h"

class UCameraComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UPrimitiveComponent;
class USpringArmComponent;
class UBWCameraSettings;

/**
 * Estado do fade de um componente que obstrui a câmera.
 * Os materiais originais ficam guardados aqui (via UPROPERTY o coletor de
 * lixo os mantém vivos enquanto durar o fade) e são restaurados quando o
 * componente deixa de obstruir.
 */
USTRUCT()
struct FBWFadedPrimitive
{
	GENERATED_BODY()

	/** Referência fraca: se o componente for destruído, a entrada é descartada sem restaurar. */
	UPROPERTY(Transient)
	TWeakObjectPtr<UPrimitiveComponent> Component;

	/** Materiais originais, um por slot do componente. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInterface>> SavedMaterials;

	/** Instância dinâmica do material de fade aplicada em todos os slots. */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> FadeMID = nullptr;

	/** Opacidade atual (1 = aparência original). */
	UPROPERTY(Transient)
	float CurrentOpacity = 1.0f;

	/** Opacidade alvo deste ciclo. */
	UPROPERTY(Transient)
	float TargetOpacity = 1.0f;

	/** O componente ainda está entre a câmera e o personagem neste passe? */
	UPROPERTY(Transient)
	bool bSeenThisPass = false;
};

/**
 * Coordena a câmera isométrica do personagem: zoom suave e fade de obstruções.
 *
 * 100% local (decisão aprovada no plano): nada aqui replica, envia RPC ou roda
 * no caminho do servidor. O gate IsLocallyControlled é refeito a cada ciclo do
 * timer, e não só no BeginPlay, porque no cliente a posse do pawn pode chegar
 * depois do BeginPlay. Pawns remotos têm o timer, mas ele sai na primeira linha
 * de UpdateCamera — custo de uma comparação por ciclo, zero rede.
 *
 * O Character chama Initialize() no construtor passando CameraBoom e
 * FollowCamera; os valores vêm do UBWCameraSettings (DA_BWCameraSettings).
 *
 * Decisões aprovadas nesta etapa: A1 (componente coordenador, CameraBoom e
 * FollowCamera preservados), B1 (zoom: roda + stick direito), C1 (canal
 * ECC_Camera), D1 (fade interpolado).
 */
UCLASS(ClassGroup = (BW), meta = (BlueprintSpawnableComponent))
class BETWEENWORLDS_API UBWCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBWCameraComponent();

	/** Conecta o componente ao braço e à câmera criados pelo Character. Chamar no construtor do Character. */
	void Initialize(USpringArmComponent* InArm, UCameraComponent* InCamera);

	/** Zoom discreto (roda do mouse). Eixo positivo aproxima (reduz o braço). */
	void AddZoomStep(float AxisValue);

	/** Zoom contínuo (analógico). Eixo positivo aproxima. Zerar ao soltar o stick. */
	void SetZoomAxisInput(float AxisValue);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Ciclo do timer local: gate de posse local, zoom e fades. */
	void UpdateCamera();

	/** Interpola o braço em direção ao alvo de zoom. */
	void UpdateZoom(float DeltaTime);

	/** Trace de oclusão (a cada 2 ciclos) e marcação de alvos de fade. */
	void UpdateOcclusion();

	/** Interpola as opacidades, aplica nos MIDs e restaura quem voltou a ficar visível. */
	void UpdateFades(float DeltaTime);

	/** Cria a entrada de fade: guarda materiais originais e aplica o MID em todos os slots. */
	FBWFadedPrimitive CreateFadeEntry(UPrimitiveComponent* Component);

	/** Devolve os materiais originais ao componente. */
	static void RestoreMaterials(UPrimitiveComponent* Component, const FBWFadedPrimitive& Entry);

	// ----- Referências criadas pelo Character -----

	UPROPERTY(VisibleAnywhere, Category = "BW|Camera")
	TObjectPtr<USpringArmComponent> Arm = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "BW|Camera")
	TObjectPtr<UCameraComponent> Camera = nullptr;

	/** Todos os valores da câmera. Configurar o DA no BP_BWPlayerCharacter. */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Camera")
	TObjectPtr<UBWCameraSettings> Settings = nullptr;

	// ----- Estado de fade (materiais vivos via UPROPERTY) -----

	UPROPERTY(Transient)
	TArray<FBWFadedPrimitive> FadedPrimitives;

	// ----- Estado de zoom e timer (não referenciam objetos; não precisam de UPROPERTY) -----

	/** Comprimento do braço que o zoom vai alcançar. */
	float ZoomTarget = 0.0f;

	/** Comprimento atual, interpolado a cada ciclo. */
	float CurrentArmLength = 0.0f;

	/** Valor do eixo analógico neste instante (0 quando solto). */
	float ZoomAxisInput = 0.0f;

	/** Período do timer local (metade do intervalo de trace). */
	float UpdateInterval = 0.05f;

	FTimerHandle UpdateTimerHandle;

	/** Alterna os ciclos: trace a cada 2 chamadas = FadeTraceInterval. */
	bool bOcclusionCycle = false;

	/** Loga a ativação uma única vez por pawn, só na máquina que o controla. */
	bool bLoggedActivation = false;

	/** Evita repetir o aviso de FadeMaterial ausente a cada trace. */
	bool bWarnedNoFadeMaterial = false;
};

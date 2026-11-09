// Between Worlds — Etapa 1.4-a (clique-para-mover)

#pragma once

#include "CoreMinimal.h"
#include "AI/Navigation/NavigationTypes.h"
#include "Components/ActorComponent.h"
#include "BWPathFollowComponent.generated.h"

class APawn;

/**
 * Segue um caminho calculado pelo NavigationSystem, NO CLIENTE LOCAL (pawn controlado localmente).
 *
 * Etapa 1.4-a: alimenta o movimento por clique (estilo isométrico). Roda somente no
 * pawn controlado localmente (cliente remoto ou host do listen server) e alimenta
 * AddMovementInput, exatamente como o WASD. Assim o movimento passa pela predição do
 * CharacterMovementComponent e o servidor continua com autoridade sobre a posição.
 * (Mover pelo servidor não funciona para clientes: o servidor ignora input de pawns
 * controlados remotamente, cujo movimento vem dos ServerMove do próprio cliente.)
 *
 * Requer Project Settings > Navigation System > "Allow Client Side Navigation" = true,
 * para que a navmesh exista também nos clientes.
 *
 * O Tick começa DESLIGADO e só liga enquanto há caminho em curso.
 *
 * Decisão B1 aprovada: destino fora da área navegável ou sem caminho possível
 * => o pedido é ignorado (nada acontece, sem teleporte nem ajuste).
 */
UCLASS(ClassGroup = (Betweenworlds), meta = (BlueprintSpawnableComponent))
class BETWEENWORLDS_API UBWPathFollowComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBWPathFollowComponent();

	/**
	 * Calcula e começa a seguir um caminho até Destination (somente pawn local).
	 * Ignora o pedido se: o pawn não é controlado localmente, o destino não cai na
	 * navmesh (B1) ou não existe caminho até ele.
	 */
	void StartPathFollowing(const FVector& Destination);

	/** Interrompe o caminho em andamento. Idempotente. */
	void StopPathFollowing();

	/** True enquanto há um caminho sendo seguido. */
	bool IsFollowingPath() const { return bFollowing; }

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Segue o waypoint atual; avança de segmento e para ao chegar. Chamado a cada Tick com caminho ativo. */
	void FollowWaypoints();

	/** Distância (2D) para aceitar um waypoint intermediário e passar ao próximo. */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Path")
	float WaypointAcceptanceRadius = 50.0f;

	/** Distância (2D) para considerar o destino alcançado e parar. */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Path")
	float DestinationAcceptanceRadius = 25.0f;

	/**
	 * Meia-extensão da busca por um ponto navegável ao validar o destino (B1).
	 * Clique até 100 uu de distância da navmesh (ex.: borda de calçada) ainda é válido;
	 * além disso, o clique é ignorado.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Path")
	float NavProjectionExtent = 100.0f;

	/**
	 * Waypoints do caminho atual (posição inicial já descartada; só tem conteúdo com bFollowing).
	 * Sem UPROPERTY de propósito: é estado temporário de runtime, não referencia UObjects
	 * (não precisa de GC), não é editado nem salvo, e assim o UHT não precisa conhecer FNavLocation.
	 */
	TArray<FNavLocation> PathPoints;

	/** Índice do waypoint que estamos alcançando agora. */
	int32 NextWaypointIndex = INDEX_NONE;

	bool bFollowing = false;
};
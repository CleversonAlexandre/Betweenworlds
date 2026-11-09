// Between Worlds — Etapa 1.4-a (clique-para-mover)

#include "Movement/BWPathFollowComponent.h"

#include "GameFramework/Pawn.h"
#include "NavigationData.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogBWPathFollow, Log, All);

UBWPathFollowComponent::UBWPathFollowComponent()
{
	// Tick sob demanda: começa desligado e é ligado em StartPathFollowing.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UBWPathFollowComponent::StartPathFollowing(const FVector& Destination)
{
	// Só o pawn controlado localmente segue caminho: o input local passa pela
	// predição do CharacterMovement e o servidor valida/replica a posição.
	APawn* ControlledPawn = Cast<APawn>(GetOwner());
	if (!ControlledPawn || !ControlledPawn->IsLocallyControlled())
	{
		return;
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		UE_LOG(LogBWPathFollow, Warning,
			TEXT("%s: NavigationSystem ausente. Verifique o NavMeshBoundsVolume no mapa e, em cliente, "
				"Project Settings > Navigation System > Allow Client Side Navigation."),
			*GetNameSafe(GetOwner()));
		return;
	}

	// B1: o destino precisa cair na navmesh; senão o pedido é ignorado.
	FNavLocation ProjectedDestination;
	if (!NavSys->ProjectPointToNavigation(Destination, ProjectedDestination, FVector(NavProjectionExtent)))
	{
		UE_LOG(LogBWPathFollow, Log,
			TEXT("%s: destino fora da área navegável, pedido ignorado (B1): %s"),
			*GetNameSafe(GetOwner()), *Destination.ToCompactString());
		return;
	}

	UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(
		GetWorld(), ControlledPawn->GetActorLocation(), ProjectedDestination.Location, ControlledPawn);

	// Os pontos ficam no FNavigationPath interno (NavPath->GetPath()), não no UNavigationPath.
	if (!NavPath || !NavPath->IsValid() || !NavPath->GetPath().IsValid())
	{
		UE_LOG(LogBWPathFollow, Log,
			TEXT("%s: sem caminho possível até o destino, pedido ignorado (B1): %s"),
			*GetNameSafe(GetOwner()), *Destination.ToCompactString());
		return;
	}

	const TArray<FNavPathPoint>& NavPoints = NavPath->GetPath()->GetPathPoints();
	if (NavPoints.Num() < 2)
	{
		return;
	}

	// Copia os waypoints (FNavPathPoint herda de FNavLocation). O primeiro
	// ponto é a posição atual: começamos pelo segundo.
	PathPoints.Reset(NavPoints.Num());
	for (const FNavPathPoint& NavPoint : NavPoints)
	{
		PathPoints.Add(NavPoint);
	}
	NextWaypointIndex = 1;

	bFollowing = true;
	SetComponentTickEnabled(true);
}

void UBWPathFollowComponent::StopPathFollowing()
{
	if (!bFollowing)
	{
		return;
	}

	bFollowing = false;
	PathPoints.Reset();
	NextWaypointIndex = INDEX_NONE;
	SetComponentTickEnabled(false);

	// O CharacterMovement consome o input a cada frame; ao parar de alimentar,
	// o personagem desacelera naturalmente.
}

void UBWPathFollowComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bFollowing)
	{
		FollowWaypoints();
	}
	else
	{
		SetComponentTickEnabled(false);
	}
}

void UBWPathFollowComponent::FollowWaypoints()
{
	APawn* ControlledPawn = Cast<APawn>(GetOwner());
	// Pawn trocou de dono (respawn/possess) ou deixou de ser local: encerra.
	if (!ControlledPawn || !ControlledPawn->IsLocallyControlled())
	{
		StopPathFollowing();
		return;
	}

	if (!PathPoints.IsValidIndex(NextWaypointIndex))
	{
		StopPathFollowing();
		return;
	}

	const FNavLocation& Target = PathPoints[NextWaypointIndex];
	const bool bIsLast = (NextWaypointIndex == PathPoints.Num() - 1);
	const float AcceptanceRadius = bIsLast ? DestinationAcceptanceRadius : WaypointAcceptanceRadius;

	FVector ToTarget = Target.Location - ControlledPawn->GetActorLocation();
	ToTarget.Z = 0.0; // Aceitação em 2D: degraus pequenos não travam o avanço.

	if (ToTarget.Size2D() <= AcceptanceRadius)
	{
		if (bIsLast)
		{
			StopPathFollowing();
			return;
		}
		++NextWaypointIndex;
		return;
	}

	// Mesma via do WASD: input local -> predição do CharacterMovement -> servidor valida.
	ControlledPawn->AddMovementInput(ToTarget.GetSafeNormal(), 1.0f);
}

void UBWPathFollowComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopPathFollowing();
	Super::EndPlay(EndPlayReason);
}
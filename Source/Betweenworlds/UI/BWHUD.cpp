// Between Worlds — Etapa 1.7 (HUD: painel de status)

#include "UI/BWHUD.h"

#include "AbilitySystemComponent.h"
#include "Core/BWPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "UI/BWStatusPanelWidget.h"

DEFINE_LOG_CATEGORY_STATIC(LogBWHUD, Log, All);

void ABWHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	if (!StatusPanelClass)
	{
		UE_LOG(LogBWHUD, Warning, TEXT("%s: StatusPanelClass não configurado. Defina WBP_StatusPanel no BP_BWHUD."),
			*GetNameSafe(this));
		return;
	}

	StatusPanel = CreateWidget<UBWStatusPanelWidget>(PC, StatusPanelClass);
	if (!StatusPanel)
	{
		return;
	}
	StatusPanel->AddToViewport();

	TryBindStatusPanel();
	if (!bStatusPanelBound)
	{
		// Ainda não ligou (PlayerState não chegou): tenta de novo em intervalos curtos.
		GetWorldTimerManager().SetTimer(BindTimerHandle, this, &ThisClass::TryBindStatusPanel, BindRetryInterval, /*bLoop=*/true);
	}
}

void ABWHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(BindTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void ABWHUD::TryBindStatusPanel()
{
	if (!StatusPanel)
	{
		GetWorldTimerManager().ClearTimer(BindTimerHandle);
		return;
	}

	const APlayerController* PC = GetOwningPlayerController();
	const ABWPlayerState* PS = PC ? PC->GetPlayerState<ABWPlayerState>() : nullptr;
	UAbilitySystemComponent* ASC = PS ? PS->GetAbilitySystemComponent() : nullptr;
	if (!ASC)
	{
		return; // Tenta de novo no próximo disparo do timer.
	}

	StatusPanel->InitializeWithAbilitySystem(ASC);
	bStatusPanelBound = true;
	GetWorldTimerManager().ClearTimer(BindTimerHandle);
	UE_LOG(LogBWHUD, Log, TEXT("%s: painel de status ligado ao ASC de %s"), *GetNameSafe(this), *GetNameSafe(PS));
}

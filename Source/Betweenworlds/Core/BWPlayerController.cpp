// Between Worlds — Etapa 1.2 (Fundação: rede)

#include "Core/BWPlayerController.h"

#include "Camera/PlayerCameraManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

DEFINE_LOG_CATEGORY_STATIC(LogBWPlayerController, Log, All);

void ABWPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// No servidor existem controllers de jogadores remotos; eles não têm LocalPlayer
	// nem input. Só o controller local registra mapeamentos.
	if (!IsLocalController())
	{
		return;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!InputSubsystem)
	{
		return;
	}

	if (DirectControlMappingContext)
	{
		InputSubsystem->AddMappingContext(DirectControlMappingContext, DirectControlMappingPriority);
	}
	else
	{
		UE_LOG(LogBWPlayerController, Warning,
			TEXT("%s: DirectControlMappingContext não configurado. Defina-o no BP_BWPlayerController."),
			*GetNameSafe(this));
	}
}

void ABWPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInput)
	{
		UE_LOG(LogBWPlayerController, Error,
			TEXT("InputComponent não é UEnhancedInputComponent. Verifique Project Settings > Input > Default Input Component Class."));
		return;
	}

	if (MoveAction)
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::HandleMove);
	}
	else
	{
		UE_LOG(LogBWPlayerController, Warning,
			TEXT("%s: MoveAction não configurada. Defina-a no BP_BWPlayerController."),
			*GetNameSafe(this));
	}
}

void ABWPlayerController::HandleMove(const FInputActionValue& Value)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || !PlayerCameraManager)
	{
		return;
	}

	const FVector2D Input = Value.Get<FVector2D>();

	// Direção relativa à câmera (apenas o giro horizontal), para que "cima" no teclado
	// ou no analógico seja sempre "para frente na tela", qualquer que seja a câmera.
	const FRotator CameraYaw(0.0, PlayerCameraManager->GetCameraRotation().Yaw, 0.0);
	const FRotationMatrix YawMatrix(CameraYaw);
	const FVector Forward = YawMatrix.GetUnitAxis(EAxis::X);
	const FVector Right = YawMatrix.GetUnitAxis(EAxis::Y);

	// AddMovementInput passa pelo CharacterMovementComponent, que já faz predição
	// no cliente e correção pelo servidor. Nenhuma replicação manual é necessária.
	ControlledPawn->AddMovementInput(Forward, Input.Y);
	ControlledPawn->AddMovementInput(Right, Input.X);
}

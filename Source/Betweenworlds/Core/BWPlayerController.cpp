// Between Worlds — Etapa 1.2 (Fundação: rede) / 1.3 (câmera) / 1.4-a (clique-para-mover)

#include "Core/BWPlayerController.h"

#include "Camera/BWCameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/EngineTypes.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "Movement/BWPathFollowComponent.h"

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

	// Modo inicial: teclado/mouse (padrão de PC), com o IMC do clique.
	if (MouseKeyboardMappingContext)
	{
		InputSubsystem->AddMappingContext(MouseKeyboardMappingContext, MouseKeyboardMappingPriority);
	}
	else
	{
		UE_LOG(LogBWPlayerController, Warning,
			TEXT("%s: MouseKeyboardMappingContext não configurado (IMC_MouseKeyboard). O clique-para-mover não funcionará."),
			*GetNameSafe(this));
	}
	ApplyInputDeviceMode();

	// A1: troca automática de modo pelo último dispositivo físico usado.
	if (UInputDeviceSubsystem* DeviceSubsystem = UInputDeviceSubsystem::Get())
	{
		DeviceSubsystem->OnInputHardwareDeviceChanged.AddDynamic(this, &ThisClass::HandleInputHardwareDeviceChanged);
		RefreshInputDeviceModeFromHardware();
	}
}

void ABWPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UInputDeviceSubsystem* DeviceSubsystem = UInputDeviceSubsystem::Get())
	{
		DeviceSubsystem->OnInputHardwareDeviceChanged.RemoveDynamic(this, &ThisClass::HandleInputHardwareDeviceChanged);
	}

	Super::EndPlay(EndPlayReason);
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

		// Started/Completed acompanham a INTENÇÃO (apertar/soltar): apertar cancela
		// caminho em curso; soltando, o clique volta a valer.
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Started, this, &ThisClass::HandleMoveStarted);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &ThisClass::HandleMoveEnded);
	}
	else
	{
		UE_LOG(LogBWPlayerController, Warning,
			TEXT("%s: MoveAction não configurada. Defina-a no BP_BWPlayerController."),
			*GetNameSafe(this));
	}

	if (ClickMoveAction)
	{
		EnhancedInput->BindAction(ClickMoveAction, ETriggerEvent::Started, this, &ThisClass::HandleClickMoveStarted);
	}
	else
	{
		UE_LOG(LogBWPlayerController, Warning,
			TEXT("%s: ClickMoveAction não configurada. Defina-a (IA_ClickMove) no BP_BWPlayerController."),
			*GetNameSafe(this));
	}

	if (ZoomWheelAction)
	{
		EnhancedInput->BindAction(ZoomWheelAction, ETriggerEvent::Triggered, this, &ThisClass::HandleZoomWheel);
	}
	else
	{
		UE_LOG(LogBWPlayerController, Warning,
			TEXT("%s: ZoomWheelAction não configurada. Defina-a (IA_ZoomWheel) no BP_BWPlayerController."),
			*GetNameSafe(this));
	}

	if (ZoomAxisAction)
	{
		// Triggered alimenta enquanto o stick está inclinado; Completed zera quando solta.
		EnhancedInput->BindAction(ZoomAxisAction, ETriggerEvent::Triggered, this, &ThisClass::HandleZoomAxisChanged);
		EnhancedInput->BindAction(ZoomAxisAction, ETriggerEvent::Completed, this, &ThisClass::HandleZoomAxisEnded);
	}
	else
	{
		UE_LOG(LogBWPlayerController, Warning,
			TEXT("%s: ZoomAxisAction não configurada. Defina-a (IA_ZoomAxis) no BP_BWPlayerController."),
			*GetNameSafe(this));
	}
}

// ---------------------------------------------------------------------------------
// A1: troca automática pelo último dispositivo usado.
// O UInputDeviceSubsystem da engine já rastreia o último hardware de cada usuário
// (teclado/mouse, gamepad...) e avisa quando muda. Na PIE com vários jogadores,
// cada janela tem seu próprio PlatformUserId, por isso filtramos pelo nosso.
// ---------------------------------------------------------------------------------
void ABWPlayerController::HandleInputHardwareDeviceChanged(const FPlatformUserId UserId, const FInputDeviceId /*DeviceId*/)
{
	if (!IsLocalController() || UserId != GetPlatformUserId())
	{
		return;
	}

	RefreshInputDeviceModeFromHardware();
}

void ABWPlayerController::RefreshInputDeviceModeFromHardware()
{
	UInputDeviceSubsystem* DeviceSubsystem = UInputDeviceSubsystem::Get();
	if (!DeviceSubsystem || !IsLocalController())
	{
		return;
	}

	const FHardwareDeviceIdentifier Device = DeviceSubsystem->GetMostRecentlyUsedHardwareDevice(GetPlatformUserId());
	switch (Device.PrimaryDeviceType)
	{
	case EHardwareDevicePrimaryType::Gamepad:
		SetInputDeviceMode(EBWInputDeviceMode::Gamepad);
		break;
	case EHardwareDevicePrimaryType::KeyboardAndMouse:
		SetInputDeviceMode(EBWInputDeviceMode::MouseKeyboard);
		break;
	default:
		// Tipo desconhecido (ou ainda nenhum input): mantém o modo atual.
		break;
	}
}

void ABWPlayerController::SetInputDeviceMode(EBWInputDeviceMode NewMode)
{
	if (InputDeviceMode == NewMode)
	{
		return;
	}

	InputDeviceMode = NewMode;
	ApplyInputDeviceMode();

	UE_LOG(LogBWPlayerController, Log,
		TEXT("%s: modo de entrada alterado para %s"),
		*GetNameSafe(this),
		InputDeviceMode == EBWInputDeviceMode::Gamepad ? TEXT("Controle") : TEXT("Teclado/Mouse"));

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		GetLocalPlayer() ? GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr;
	if (InputSubsystem && MouseKeyboardMappingContext)
	{
		if (InputDeviceMode == EBWInputDeviceMode::MouseKeyboard)
		{
			InputSubsystem->AddMappingContext(MouseKeyboardMappingContext, MouseKeyboardMappingPriority);
		}
		else
		{
			InputSubsystem->RemoveMappingContext(MouseKeyboardMappingContext);
		}
	}
}

void ABWPlayerController::ApplyInputDeviceMode()
{
	// Cursor visível só no modo teclado/mouse; no modo controle a mira é o personagem.
	bShowMouseCursor = (InputDeviceMode == EBWInputDeviceMode::MouseKeyboard);
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

void ABWPlayerController::HandleMoveStarted(const FInputActionValue& /*Value*/)
{
	bDirectMoveInputActive = true;

	// Movimento direto tem prioridade sobre o caminho do clique.
	if (UBWPathFollowComponent* PathFollow = GetBWPathFollowComponent())
	{
		PathFollow->StopPathFollowing();
	}
}

void ABWPlayerController::HandleMoveEnded(const FInputActionValue& /*Value*/)
{
	bDirectMoveInputActive = false;
}

void ABWPlayerController::HandleClickMoveStarted(const FInputActionValue& /*Value*/)
{
	if (!IsLocalController())
	{
		return;
	}

	// A1: clique só existe no modo teclado/mouse.
	if (InputDeviceMode != EBWInputDeviceMode::MouseKeyboard)
	{
		return;
	}

	// Enquanto o jogador dirige com WASD/stick, cliques são ignorados.
	if (bDirectMoveInputActive)
	{
		return;
	}

	// B1: sem hit (céu/espaço vazio) não há destino; pedido ignorado.
	FHitResult Hit;
	if (!GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), false, Hit))
	{
		return;
	}

	RequestMoveTo(Hit.ImpactPoint);
}

void ABWPlayerController::RequestMoveTo(const FVector& Destination)
{
	// Executa localmente (host ou cliente): o componente segue o caminho com
	// AddMovementInput e o CharacterMovement replica/valida no servidor.
	if (UBWPathFollowComponent* PathFollow = GetBWPathFollowComponent())
	{
		PathFollow->StartPathFollowing(Destination);
	}
}

UBWCameraComponent* ABWPlayerController::GetBWCameraComponent() const
{
	const APawn* ControlledPawn = GetPawn();
	return ControlledPawn ? ControlledPawn->FindComponentByClass<UBWCameraComponent>() : nullptr;
}

UBWPathFollowComponent* ABWPlayerController::GetBWPathFollowComponent() const
{
	const APawn* ControlledPawn = GetPawn();
	return ControlledPawn ? ControlledPawn->FindComponentByClass<UBWPathFollowComponent>() : nullptr;
}

void ABWPlayerController::HandleZoomWheel(const FInputActionValue& Value)
{
	if (UBWCameraComponent* BWCamera = GetBWCameraComponent())
	{
		BWCamera->AddZoomStep(Value.Get<float>());
	}
}

void ABWPlayerController::HandleZoomAxisChanged(const FInputActionValue& Value)
{
	if (UBWCameraComponent* BWCamera = GetBWCameraComponent())
	{
		BWCamera->SetZoomAxisInput(Value.Get<float>());
	}
}

void ABWPlayerController::HandleZoomAxisEnded(const FInputActionValue& /*Value*/)
{
	if (UBWCameraComponent* BWCamera = GetBWCameraComponent())
	{
		BWCamera->SetZoomAxisInput(0.0f);
	}
}
// Between Worlds — Etapa 1.2 (Fundação: rede) / 1.3 (câmera)

#include "Character/BWPlayerCharacter.h"

#include "Camera/BWCameraComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

ABWPlayerCharacter::ABWPlayerCharacter()
{
	// Câmera (coordenada pelo BWCamera a partir do DA_BWCameraSettings desde a Etapa 1.3).
	// Nomes e classes preservados: o BP_BWPlayerCharacter criado na 1.2 continua válido.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->SetRelativeRotation(FRotator(-55.0, 0.0, 0.0));
	CameraBoom->TargetArmLength = 1400.f;
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bDoCollisionTest = false; // Obstruções são tratadas com fade pelo BWCamera (não puxar a câmera).

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Coordena zoom e fade. As referências são passadas aqui; os valores vêm do Data Asset.
	BWCamera = CreateDefaultSubobject<UBWCameraComponent>(TEXT("BWCamera"));
	BWCamera->Initialize(CameraBoom, FollowCamera);
}

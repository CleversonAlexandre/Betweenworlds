// Between Worlds — Etapa 1.2 (Fundação: rede)

#include "Character/BWPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

ABWPlayerCharacter::ABWPlayerCharacter()
{
	// Câmera provisória (substituída na Etapa 1.3). Valores editáveis no BP_BWPlayerCharacter.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->SetRelativeRotation(FRotator(-55.0, 0.0, 0.0));
	CameraBoom->TargetArmLength = 1400.f;
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bDoCollisionTest = false; // Obstruções serão tratadas com fade na Etapa 1.3.

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

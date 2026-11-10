// Between Worlds — Etapa 1.2 (Fundação: rede) / 1.5 (GAS: interface com o ASC)

#include "Character/BWCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ABWCharacterBase::ABWCharacterBase()
{
	// Personagem não precisa de Tick próprio; movimento e animação têm os seus.
	PrimaryActorTick.bCanEverTick = false;

	// Em câmera isométrica o personagem vira para onde anda,
	// não para onde o controller/câmera aponta.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Valores iniciais. Ajustáveis no Blueprint filho (sem recompilar).
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = true;
		Movement->RotationRate = FRotator(0.0, 720.0, 0.0);
	}
}

UAbilitySystemComponent* ABWCharacterBase::GetAbilitySystemComponent() const
{
	// Padrão seguro: sem ASC na base. O BWPlayerCharacter sobrescreve e devolve o
	// ASC do PlayerState (TECHNICAL_DESIGN §4); inimigos terão o próprio depois.
	return nullptr;
}

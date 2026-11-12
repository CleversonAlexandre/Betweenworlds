// Between Worlds — Etapa 1.2 (Fundação: rede) / 1.5 (GAS: interface com o ASC) / 2.3 (morte) / 2.4 (times)

#include "Character/BWCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "Character/BWDeathComponent.h"
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

	// Etapa 2.3: todo personagem pode morrer. Ligado ao ASC pela classe filha
	// quando o ASC estiver pronto (jogador: InitAbilityActorInfo).
	DeathComponent = CreateDefaultSubobject<UBWDeathComponent>(TEXT("BWDeath"));
}

UAbilitySystemComponent* ABWCharacterBase::GetAbilitySystemComponent() const
{
	// Padrão seguro: sem ASC na base. O BWPlayerCharacter sobrescreve e devolve o
	// ASC do PlayerState (TECHNICAL_DESIGN §4); inimigos terão o próprio depois.
	return nullptr;
}

bool ABWCharacterBase::IsDead() const
{
	return DeathComponent && DeathComponent->IsDead();
}

bool ABWCharacterBase::IsHostileTo(const AActor* Other) const
{
	const ABWCharacterBase* OtherCharacter = Cast<ABWCharacterBase>(Other);
	return OtherCharacter && OtherCharacter != this && OtherCharacter->TeamId != TeamId && !OtherCharacter->IsDead();
}

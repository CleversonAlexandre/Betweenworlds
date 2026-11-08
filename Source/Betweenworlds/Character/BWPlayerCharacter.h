// Between Worlds — Etapa 1.2 (Fundação: rede)

#pragma once

#include "CoreMinimal.h"
#include "Character/BWCharacterBase.h"
#include "BWPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;

/**
 * Personagem controlado por jogador.
 *
 * Etapa 1.2: câmera PROVISÓRIA (braço fixo visto de cima), apenas para enxergar o teste de rede.
 * Etapa 1.3 substitui por BWCameraComponent (zoom, enquadramento, fade de obstruções).
 * Etapa 1.5 inicializa aqui o AbilitySystemComponent vindo do PlayerState.
 *
 * Abstract: o jogo usa o filho em Blueprint (BP_BWPlayerCharacter), que define malha e animação.
 */
UCLASS(Abstract)
class BETWEENWORLDS_API ABWPlayerCharacter : public ABWCharacterBase
{
	GENERATED_BODY()

public:
	ABWPlayerCharacter();

private:
	/** Braço da câmera provisória. Rotação absoluta: não gira junto com o personagem. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BW|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BW|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
};

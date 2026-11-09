// Between Worlds — Etapa 1.2 (Fundação: rede) / 1.3 (câmera) / 1.4-a (clique-para-mover)

#pragma once

#include "CoreMinimal.h"
#include "Character/BWCharacterBase.h"
#include "BWPlayerCharacter.generated.h"

class UBWCameraComponent;
class UBWPathFollowComponent;
class UCameraComponent;
class USpringArmComponent;

/**
 * Personagem controlado por jogador.
 *
 * Etapa 1.3: CameraBoom e FollowCamera continuam criados aqui, com os mesmos
 * nomes — a árvore de componentes do BP_BWPlayerCharacter criado na 1.2 não
 * muda. O UBWCameraComponent coordena zoom e fade, com os valores vindos do
 * DA_BWCameraSettings (atribuído ao componente BWCamera no Blueprint).
 * Etapa 1.4-a: BWPathFollow criado aqui; é ele quem anda o personagem até o
 * destino do clique, no servidor, autoridade. O Blueprint continua válido.
 * Etapa 1.5 inicializará aqui o AbilitySystemComponent vindo do PlayerState.
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
        /** Braço da câmera. Rotação absoluta: não gira junto com o personagem. Pitch e comprimento aplicados pelo BWCamera a partir do Data Asset. */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BW|Camera", meta = (AllowPrivateAccess = "true"))
        TObjectPtr<USpringArmComponent> CameraBoom;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BW|Camera", meta = (AllowPrivateAccess = "true"))
        TObjectPtr<UCameraComponent> FollowCamera;

        /** Coordena zoom suave e fade de obstruções (Etapa 1.3). 100% local. */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BW|Camera", meta = (AllowPrivateAccess = "true"))
        TObjectPtr<UBWCameraComponent> BWCamera;

        /** Executa caminhos da navmesh no servidor (Etapa 1.4-a). Cliente só pede o destino. */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BW|Movement", meta = (AllowPrivateAccess = "true"))
        TObjectPtr<UBWPathFollowComponent> BWPathFollow;
};

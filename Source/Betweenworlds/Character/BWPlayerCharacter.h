// Between Worlds — Etapa 1.2 (Fundação: rede) / 1.3 (câmera) / 1.4-a (clique-para-mover) / 1.5 (GAS) / 2.3 (morte e respawn)

#pragma once

#include "CoreMinimal.h"
#include "Character/BWCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BWPlayerCharacter.generated.h"

class UAbilitySystemComponent;
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
 * destino do clique, rodando no cliente local (predição do CharacterMovement)
 * enquanto o servidor valida a posição. O Blueprint continua válido.
 * Etapa 1.5: o AbilitySystemComponent vive no PlayerState (TECHNICAL_DESIGN §4);
 * este personagem é o avatar — liga o ASC a si (PossessedBy no servidor,
 * OnRep_PlayerState no cliente) e pede o grant das habilidades.
 *
 * Abstract: o jogo usa o filho em Blueprint (BP_BWPlayerCharacter), que define malha e animação.
 */
UCLASS(Abstract)
class BETWEENWORLDS_API ABWPlayerCharacter : public ABWCharacterBase
{
        GENERATED_BODY()

public:
        ABWPlayerCharacter();

        // Etapa 1.5: GAS — o ASC vive no PlayerState; este personagem é o avatar.
        virtual void PossessedBy(AController* NewController) override;
        virtual void OnRep_PlayerState() override;
        virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
        virtual void BeginPlay() override;

private:
        /** Etapa 2.3 (servidor): este corpo morreu — pede o respawn ao GameMode. */
        void HandleDeathStarted(AActor* DeadActor);

        /** Braço da câmera. Rotação absoluta: não gira junto com o personagem. Pitch e comprimento aplicados pelo BWCamera a partir do Data Asset. */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BW|Camera", meta = (AllowPrivateAccess = "true"))
        TObjectPtr<USpringArmComponent> CameraBoom;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BW|Camera", meta = (AllowPrivateAccess = "true"))
        TObjectPtr<UCameraComponent> FollowCamera;

        /** Coordena zoom suave e fade de obstruções (Etapa 1.3). 100% local. */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BW|Camera", meta = (AllowPrivateAccess = "true"))
        TObjectPtr<UBWCameraComponent> BWCamera;

        /** Executa caminhos da navmesh no cliente local (Etapa 1.4-a); o servidor valida a posição. */
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BW|Movement", meta = (AllowPrivateAccess = "true"))
        TObjectPtr<UBWPathFollowComponent> BWPathFollow;

        /** Etapa 1.5: liga o ASC do PlayerState a este avatar (Owner = PlayerState, Avatar = este personagem). */
        void InitAbilityActorInfoFromPlayerState();
};

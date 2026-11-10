// Between Worlds — Etapa 1.2 (Fundação: rede) / 1.3 (câmera) / 1.4-a (clique-para-mover) / 1.5 (GAS)

#include "Character/BWPlayerCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"

#include "AbilitySystemComponent.h"
#include "Camera/BWCameraComponent.h"
#include "Camera/CameraComponent.h"
#include "Core/BWPlayerState.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Movement/BWPathFollowComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogBWPlayerCharacter, Log, All);

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

        // Etapa 1.4-a (revisão do Claude): caminho da navmesh rodando NO CLIENTE
        // local (predição do CharacterMovement); o servidor valida a posição.
        BWPathFollow = CreateDefaultSubobject<UBWPathFollowComponent>(TEXT("BWPathFollow"));

        // Etapa 1.4-a.1 — suavização dos personagens REMOTOS (simulated proxies:
        // os pawns dos OUTROS jogadores, vistos nas janelas de cliente). Padrão da
        // engine: suaviza atraso de até 256 uu e "encaixa" seco acima de 384 uu.
        // Com 4 janelas PIE o FPS cai, as atualizações de rede ficam esparsas e o
        // personagem dos outros pisa/pula (quase some) nas outras telas — dobramos
        // as duas distâncias para a interpolação aguentar atualizações esparsas:
        GetCharacterMovement()->NetworkMaxSmoothUpdateDistance = 512.f;
        GetCharacterMovement()->NetworkNoSmoothUpdateDistance = 1024.f;
        // Listen server: o host também é o servidor; reduzir a taxa de replicação
        // por pawn alivia a game thread dele (gargalo do lag reportado). 30
        // atualizações/s com os buffers acima continuam suaves a 4 jogadores:
        NetUpdateFrequency = 30.f;
}

// ---------------------------------------------------------------------------------
// Etapa 1.5 — GAS (TECHNICAL_DESIGN §4): o AbilitySystemComponent vive no
// PlayerState; este personagem é o AVATAR. Servidor: PossessedBy liga o ASC e
// pede o grant. Cliente: OnRep_PlayerState liga o ASC replicado ao avatar local.
// ---------------------------------------------------------------------------------
void ABWPlayerCharacter::PossessedBy(AController* NewController)
{
        Super::PossessedBy(NewController);

        InitAbilityActorInfoFromPlayerState();

        // Concede as habilidades configuradas no BP_BWPlayerState (guard interno:
        // servidor + uma vez por PlayerState — respawn não reconcede).
        if (ABWPlayerState* BWPlayerState = GetPlayerState<ABWPlayerState>())
        {
                BWPlayerState->GrantDefaultAbilities();
        }
}

void ABWPlayerCharacter::OnRep_PlayerState()
{
        Super::OnRep_PlayerState();

        // Cliente: o PlayerState replicado chegou — liga o ASC a este avatar.
        InitAbilityActorInfoFromPlayerState();
}

UAbilitySystemComponent* ABWPlayerCharacter::GetAbilitySystemComponent() const
{
        const ABWPlayerState* BWPlayerState = GetPlayerState<ABWPlayerState>();
        return BWPlayerState ? BWPlayerState->GetAbilitySystemComponent() : nullptr;
}

void ABWPlayerCharacter::InitAbilityActorInfoFromPlayerState()
{
        ABWPlayerState* BWPlayerState = GetPlayerState<ABWPlayerState>();
        if (!BWPlayerState)
        {
                UE_LOG(LogBWPlayerCharacter, Warning,
                        TEXT("%s: PlayerState não é BWPlayerState — confira o PlayerStateClass no BP_BWGameMode (Etapa 1.5)."),
                        *GetNameSafe(this));
                return;
        }

        UAbilitySystemComponent* AbilitySystem = BWPlayerState->GetAbilitySystemComponent();
        if (!AbilitySystem)
        {
                UE_LOG(LogBWPlayerCharacter, Warning,
                        TEXT("%s: BWPlayerState sem AbilitySystemComponent."),
                        *GetNameSafe(this));
                return;
        }

        // Owner = PlayerState (dono do ASC e da lista de habilidades); Avatar = este
        // personagem (quem executa). Servidor: ativação autoritativa; cliente: predição.
        AbilitySystem->InitAbilityActorInfo(BWPlayerState, this);
}

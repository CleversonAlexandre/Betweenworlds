// Between Worlds — Etapa 1.2 (Fundação: rede) / 1.3 (câmera) / 1.4-a (clique-para-mover) / 1.4-b (InputTags) / 1.5 (GAS)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h" // FGameplayTag das habilidades — no .h: o UHT lê este arquivo isolado (lição do erro #3)
#include "BWPlayerController.generated.h"

class UBWCameraComponent;
class UBWInputConfig;
class UBWPathFollowComponent;
class UInputAction;
class UInputMappingContext;
class UEnhancedInputComponent;
struct FInputActionValue;

/** Dispositivo usado por último pelo jogador local (decisão A1 da Etapa 1.4-a). */
enum class EBWInputDeviceMode : uint8
{
        MouseKeyboard,
        Gamepad
};

/**
 * Controller do jogador. Existe no servidor e no cliente dono; input só roda no cliente local.
 *
 * Etapa 1.2: movimento direto (WASD / analógico), relativo à câmera.
 * Etapa 1.3: zoom da câmera (roda do mouse e analógico), repassado ao BWCameraComponent.
 * Etapa 1.4-a: clique-para-mover. O caminho é calculado e seguido NO CLIENTE LOCAL
 * (BWPathFollowComponent + AddMovementInput), passando pela predição do
 * CharacterMovementComponent — o servidor continua com autoridade sobre a posição.
 * Troca automática do modo de entrada pelo último dispositivo usado (A1),
 * detectada pelo UInputDeviceSubsystem da engine.
 * Etapa 1.4-b: roteamento botão → InputTag (UBWInputConfig).
 * Etapa 1.5: repassa "tag pressionada/solta" ao AbilitySystemComponent do
 * PlayerState (TECHNICAL_DESIGN §4) — o ASC ativa a habilidade do slot.
 */
UCLASS()
class BETWEENWORLDS_API ABWPlayerController : public APlayerController
{
        GENERATED_BODY()

protected:
        virtual void BeginPlay() override;
        virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
        virtual void SetupInputComponent() override;

private:
        /** Converte o eixo 2D de input em movimento no plano, relativo à direção da câmera. */
        void HandleMove(const FInputActionValue& Value);

        /** Primeiro input direto do movimento (tecla ou stick): cancela caminho em curso. */
        void HandleMoveStarted(const FInputActionValue& Value);

        /** Movimento direto solto: marca que o clique pode voltar a valer. */
        void HandleMoveEnded(const FInputActionValue& Value);

        /** Clique esquerdo (IA_ClickMove): trace sob o cursor e início do caminho local. */
        void HandleClickMoveStarted(const FInputActionValue& Value);

        /** Zoom discreto (roda do mouse): um passo por clique. */
        void HandleZoomWheel(const FInputActionValue& Value);

        /** Zoom contínuo (analógico): alimenta o componente enquanto o stick está inclinado. */
        void HandleZoomAxisChanged(const FInputActionValue& Value);

        /** Zera a entrada analógica quando o stick é solto. */
        void HandleZoomAxisEnded(const FInputActionValue& Value);

        /** Faz o bind de cada ação com tag do BWInputConfig → AbilityInputTagPressed/Released (Etapa 1.4-b). */
        void BindTaggedInputActions(UEnhancedInputComponent* EnhancedInput);

        /**
         * Slot de habilidade pressionado (InputTag.Ability.1 / .2).
         * Etapa 1.5: loga e repassa a tag ao AbilitySystemComponent do PlayerState
         * (TECHNICAL_DESIGN §4) — sem delegates.
         */
        void AbilityInputTagPressed(FGameplayTag InputTag);

        /** Slot de habilidade solto. Etapa 1.5: loga e repassa ao ASC do PlayerState. */
        void AbilityInputTagReleased(FGameplayTag InputTag);

        /** Componente de câmera do pawn possuído (ou null). */
        UBWCameraComponent* GetBWCameraComponent() const;

        /** Componente de caminho do pawn possuído (ou null). */
        UBWPathFollowComponent* GetBWPathFollowComponent() const;

        /** Começa a seguir um caminho até o destino (somente no controller local). */
        void RequestMoveTo(const FVector& Destination);

        /**
         * Chamado pela engine (UInputDeviceSubsystem) quando um usuário troca de dispositivo
         * físico. Substitui o override de APlayerController::InputKey, que é 'final' na 5.7.
         */
        UFUNCTION()
        void HandleInputHardwareDeviceChanged(const FPlatformUserId UserId, const FInputDeviceId DeviceId);

        /** Lê o último dispositivo do usuário local e aplica o modo correspondente. */
        void RefreshInputDeviceModeFromHardware();

        /** Aplica o modo de entrada novo, se mudou: troca o IMC do clique e o cursor. */
        void SetInputDeviceMode(EBWInputDeviceMode NewMode);

        /** Efeitos visuais do modo atual (cursor). */
        void ApplyInputDeviceMode();

        /** Mapeamento para WASD e controle (IMC_DirectControl). */
        UPROPERTY(EditDefaultsOnly, Category = "BW|Input")
        TObjectPtr<UInputMappingContext> DirectControlMappingContext;

        /** Prioridade do mapeamento acima. Maior vence quando dois contextos usam a mesma tecla. */
        UPROPERTY(EditDefaultsOnly, Category = "BW|Input")
        int32 DirectControlMappingPriority = 0;

        /**
         * Mapeamento do modo teclado/mouse (IMC_MouseKeyboard), com o clique-para-mover.
         * Adicionado/removido conforme o último dispositivo usado (A1).
         */
        UPROPERTY(EditDefaultsOnly, Category = "BW|Input")
        TObjectPtr<UInputMappingContext> MouseKeyboardMappingContext;

        /** Prioridade do mapeamento acima. */
        UPROPERTY(EditDefaultsOnly, Category = "BW|Input")
        int32 MouseKeyboardMappingPriority = 1;

        /** Ação de movimento, tipo Axis2D (IA_Move). */
        UPROPERTY(EditDefaultsOnly, Category = "BW|Input")
        TObjectPtr<UInputAction> MoveAction;

        /** Ação do clique-para-mover, tipo Bool (IA_ClickMove, botão esquerdo). */
        UPROPERTY(EditDefaultsOnly, Category = "BW|Input")
        TObjectPtr<UInputAction> ClickMoveAction;

        /** Ação de zoom da roda do mouse, tipo Axis1D (IA_ZoomWheel). */
        UPROPERTY(EditDefaultsOnly, Category = "BW|Input")
        TObjectPtr<UInputAction> ZoomWheelAction;

        /** Ação de zoom do analógico, tipo Axis1D (IA_ZoomAxis). */
        UPROPERTY(EditDefaultsOnly, Category = "BW|Input")
        TObjectPtr<UInputAction> ZoomAxisAction;

        /** Roteamento botão → InputTag (DA_BWInputConfig, Etapa 1.4-b). Só habilidades: os inputs nativos (WASD, clique, zoom) ficam fora — decisão B1. */
        UPROPERTY(EditDefaultsOnly, Category = "BW|Input")
        TObjectPtr<UBWInputConfig> BWInputConfig;

        /** Modo de entrada atual (local, não replicado). */
        EBWInputDeviceMode InputDeviceMode = EBWInputDeviceMode::MouseKeyboard;

        /** True enquanto WASD/stick está pressionado: clique é ignorado (comportamento de ARPG com WASD). */
        bool bDirectMoveInputActive = false;
};

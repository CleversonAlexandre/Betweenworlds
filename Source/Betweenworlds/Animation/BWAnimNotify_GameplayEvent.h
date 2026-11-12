// Between Worlds — Etapa 2.4 (acerto corpo a corpo)

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "BWAnimNotify_GameplayEvent.generated.h"

/**
 * Notify de animação que envia um Gameplay Event ao dono da malha (padrão: Event.Montage.Hit).
 * Colocado no quadro de acerto dos montages de ataque; a habilidade que tocou o montage
 * espera esse evento para aplicar o golpe (GAS_ARCHITECTURE §10: "janelas de acerto via
 * Gameplay Event disparado por notify").
 *
 * Roda em todas as máquinas que tocam a animação; só o servidor aplica dano.
 */
UCLASS(meta = (DisplayName = "BW Gameplay Event"))
class BETWEENWORLDS_API UBWAnimNotify_GameplayEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	UBWAnimNotify_GameplayEvent();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

	/** Evento enviado. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BW", meta = (Categories = "Event"))
	FGameplayTag EventTag;
};

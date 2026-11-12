// Between Worlds — Etapa 2.4 (acerto corpo a corpo)

#include "Animation/BWAnimNotify_GameplayEvent.h"

#include "AbilitySystem/Tags/BWCombatTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"

UBWAnimNotify_GameplayEvent::UBWAnimNotify_GameplayEvent()
{
	EventTag = TAG_Event_Montage_Hit;
}

void UBWAnimNotify_GameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!Owner || !EventTag.IsValid())
	{
		return; // Ex.: preview do editor de animação, sem personagem.
	}

	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	Payload.Instigator = Owner;

	// Jogador: o personagem devolve o ASC do PlayerState; inimigo: o ASC próprio.
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, Payload);
}

FString UBWAnimNotify_GameplayEvent::GetNotifyName_Implementation() const
{
	return EventTag.IsValid() ? EventTag.ToString() : TEXT("BW Gameplay Event");
}

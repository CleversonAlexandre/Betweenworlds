// Between Worlds — Etapa 1.4-b (InputTags: infra para as habilidades da 1.5)

#include "Input/BWInputConfig.h"

#include "InputAction.h"

DEFINE_LOG_CATEGORY_STATIC(LogBWInputConfig, Log, All);

UInputAction* UBWInputConfig::FindInputActionForTag(const FGameplayTag& Tag) const
{
	for (const FBWInputAction& Entry : TaggedInputActions)
	{
		if (Entry.InputAction && Entry.InputTag == Tag)
		{
			return Entry.InputAction;
		}
	}

	UE_LOG(LogBWInputConfig, Warning,
		TEXT("%s: nenhuma Input Action com a tag \"%s\" — confira o DA_BWInputConfig."),
		*GetNameSafe(this), *Tag.ToString());
	return nullptr;
}

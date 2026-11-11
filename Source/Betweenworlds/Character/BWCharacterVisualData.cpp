// Between Worlds — Etapa 2.1 (Visual por Data Asset)

#include "Character/BWCharacterVisualData.h"

#include "Animation/AnimMontage.h"

DEFINE_LOG_CATEGORY_STATIC(LogBWVisualData, Log, All);

UAnimMontage* UBWCharacterVisualData::LoadMontage(const FGameplayTag& MontageTag) const
{
	const TSoftObjectPtr<UAnimMontage>* Entry = Montages.Find(MontageTag);
	if (!Entry || Entry->IsNull())
	{
		UE_LOG(LogBWVisualData, Warning, TEXT("%s: sem montage para a tag %s."), *GetName(), *MontageTag.ToString());
		return nullptr;
	}

	UAnimMontage* Montage = Entry->LoadSynchronous();
	if (!Montage)
	{
		UE_LOG(LogBWVisualData, Warning, TEXT("%s: montage da tag %s não encontrado (%s). Pack removido?"),
			*GetName(), *MontageTag.ToString(), *Entry->ToString());
	}
	return Montage;
}

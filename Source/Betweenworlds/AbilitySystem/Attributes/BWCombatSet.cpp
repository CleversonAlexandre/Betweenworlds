// Between Worlds — Etapa 2.7 (atributos de combate)

#include "AbilitySystem/Attributes/BWCombatSet.h"

#include "Net/UnrealNetwork.h"

UBWCombatSet::UBWCombatSet()
{
	InitDamageMultiplier(1.0f);
}

void UBWCombatSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UBWCombatSet, DamageMultiplier, COND_None, REPNOTIFY_Always);
}

void UBWCombatSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetDamageMultiplierAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UBWCombatSet::OnRep_DamageMultiplier(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBWCombatSet, DamageMultiplier, OldValue);
}

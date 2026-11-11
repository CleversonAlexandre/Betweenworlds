// Between Worlds — Etapa 2.2 (Fúria e dano)

#include "AbilitySystem/Attributes/BWFurySet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UBWFurySet::UBWFurySet()
{
	// Valores reais vêm do GE_Warrior_InitFury (dados, não código).
	InitFury(0.0f);
	InitMaxFury(1.0f);
}

void UBWFurySet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UBWFurySet, Fury, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBWFurySet, MaxFury, COND_None, REPNOTIFY_Always);
}

void UBWFurySet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetFuryAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxFury());
	}
	else if (Attribute == GetMaxFuryAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void UBWFurySet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Ganho (instantâneo) e decaimento (periódico) alteram o valor base: limita aqui.
	if (Data.EvaluatedData.Attribute == GetFuryAttribute())
	{
		SetFury(FMath::Clamp(GetFury(), 0.0f, GetMaxFury()));
	}
}

void UBWFurySet::OnRep_Fury(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBWFurySet, Fury, OldValue);
}

void UBWFurySet::OnRep_MaxFury(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBWFurySet, MaxFury, OldValue);
}

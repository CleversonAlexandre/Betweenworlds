// Between Worlds — Etapa 1.6 (Atributos e Gameplay Effects) / 2.2 (evento de dano recebido) / 2.3 (morte)

#include "AbilitySystem/Attributes/BWHealthSet.h"

#include "AbilitySystem/Tags/BWCombatTags.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogBWHealthSet, Log, All);

UBWHealthSet::UBWHealthSet()
{
	// Valores reais vêm do GE_InitAttributes (dados, não código).
	InitHealth(1.0f);
	InitMaxHealth(1.0f);
	InitIncomingDamage(0.0f);
}

void UBWHealthSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// REPNOTIFY_Always: o OnRep roda mesmo se o valor não mudou, o que a predição do GAS exige.
	DOREPLIFETIME_CONDITION_NOTIFY(UBWHealthSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBWHealthSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UBWHealthSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Limite do valor "atual" (efeitos com duração). Instantâneos são tratados abaixo.
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void UBWHealthSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		// Consome o meta-atributo: ele é só um "envelope" do dano.
		const float Damage = GetIncomingDamage();
		SetIncomingDamage(0.0f);

		// Morto não recebe dano (nem gera Fúria, nem morre de novo).
		UAbilitySystemComponent* OwnerASC = GetOwningAbilitySystemComponent();
		if (OwnerASC && OwnerASC->HasMatchingGameplayTag(TAG_State_Dead))
		{
			return;
		}

		if (Damage > 0.0f)
		{
			const float OldHealth = GetHealth();
			const float NewHealth = FMath::Clamp(GetHealth() - Damage, 0.0f, GetMaxHealth());
			SetHealth(NewHealth);

			UE_LOG(LogBWHealthSet, Log, TEXT("%s: recebeu %.1f de dano -> Health %.1f / %.1f"),
				*GetNameSafe(GetOwningActor()), Damage, NewHealth, GetMaxHealth());

			// Avisa quem quiser reagir a dano recebido (Fúria do Guerreiro, futuramente
			// reação a golpe, passivas, lendários) sem que este set conheça nenhum deles.
			if (UAbilitySystemComponent* ASC = OwnerASC)
			{
				FGameplayEventData Payload;
				Payload.EventTag = TAG_Event_Damage_Received;
				Payload.EventMagnitude = Damage;
				Payload.Instigator = Data.EffectSpec.GetContext().GetOriginalInstigator();
				Payload.Target = ASC->GetAvatarActor();
				Payload.ContextHandle = Data.EffectSpec.GetContext();
				ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
			}

			// Morte: só na passagem de "vivo" para 0. Quem trata é o BWDeathComponent do corpo.
			if (NewHealth <= 0.0f && OldHealth > 0.0f && OwnerASC)
			{
				FGameplayEventData DeathPayload;
				DeathPayload.EventTag = TAG_Event_Death;
				DeathPayload.Instigator = Data.EffectSpec.GetContext().GetOriginalInstigator();
				DeathPayload.Target = OwnerASC->GetAvatarActor();
				DeathPayload.ContextHandle = Data.EffectSpec.GetContext();
				OwnerASC->HandleGameplayEvent(DeathPayload.EventTag, &DeathPayload);
			}
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
}

void UBWHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBWHealthSet, Health, OldValue);
}

void UBWHealthSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBWHealthSet, MaxHealth, OldValue);
}

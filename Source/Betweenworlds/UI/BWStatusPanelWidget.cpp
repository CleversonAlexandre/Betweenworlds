// Between Worlds — Etapa 1.7 (HUD: painel de status) / 2.2 (recurso da classe)

#include "UI/BWStatusPanelWidget.h"

#include "AbilitySystem/Attributes/BWFurySet.h"
#include "AbilitySystem/Attributes/BWHealthSet.h"
#include "AbilitySystem/Attributes/BWResourceSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"

namespace BWStatusPanel
{
	/** Atributos que o painel acompanha (atuais e máximos). */
	static TArray<FGameplayAttribute> GetWatchedAttributes()
	{
		return {
			UBWHealthSet::GetHealthAttribute(),
			UBWHealthSet::GetMaxHealthAttribute(),
			UBWResourceSet::GetManaAttribute(),
			UBWResourceSet::GetMaxManaAttribute(),
			UBWResourceSet::GetStaminaAttribute(),
			UBWResourceSet::GetMaxStaminaAttribute(),
			UBWFurySet::GetFuryAttribute(),
			UBWFurySet::GetMaxFuryAttribute()
		};
	}
}

void UBWStatusPanelWidget::InitializeWithAbilitySystem(UAbilitySystemComponent* InAbilitySystem)
{
	UnbindFromAbilitySystem();

	AbilitySystem = InAbilitySystem;
	if (!InAbilitySystem)
	{
		return;
	}

	for (const FGameplayAttribute& Attribute : BWStatusPanel::GetWatchedAttributes())
	{
		InAbilitySystem->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &ThisClass::HandleAttributeChanged);
	}

	// Valores atuais já na abertura; daqui em diante só por eventos.
	BroadcastAll();
}

void UBWStatusPanelWidget::NativeDestruct()
{
	UnbindFromAbilitySystem();
	Super::NativeDestruct();
}

void UBWStatusPanelWidget::HandleAttributeChanged(const FOnAttributeChangeData& /*Data*/)
{
	// Um atributo pode mudar sem o par (ex.: só o máximo). Reenviar tudo é barato
	// e mantém o Blueprint simples (sempre recebe atual + máximo juntos).
	// Também cobre o set de Fúria chegando depois pela rede: a primeira mudança dele
	// já faz a segunda barra trocar de Mana para Fúria.
	BroadcastAll();
}

void UBWStatusPanelWidget::BroadcastAll()
{
	const UAbilitySystemComponent* ASC = AbilitySystem.Get();
	if (!ASC)
	{
		return;
	}

	OnHealthChanged(
		ASC->GetNumericAttribute(UBWHealthSet::GetHealthAttribute()),
		ASC->GetNumericAttribute(UBWHealthSet::GetMaxHealthAttribute()));

	// Recurso da classe: Fúria se o set existir (Guerreiro), senão Mana.
	if (ASC->GetSet<UBWFurySet>())
	{
		OnClassResourceChanged(
			ASC->GetNumericAttribute(UBWFurySet::GetFuryAttribute()),
			ASC->GetNumericAttribute(UBWFurySet::GetMaxFuryAttribute()),
			EBWClassResource::Fury);
	}
	else
	{
		OnClassResourceChanged(
			ASC->GetNumericAttribute(UBWResourceSet::GetManaAttribute()),
			ASC->GetNumericAttribute(UBWResourceSet::GetMaxManaAttribute()),
			EBWClassResource::Mana);
	}

	OnStaminaChanged(
		ASC->GetNumericAttribute(UBWResourceSet::GetStaminaAttribute()),
		ASC->GetNumericAttribute(UBWResourceSet::GetMaxStaminaAttribute()));
}

void UBWStatusPanelWidget::UnbindFromAbilitySystem()
{
	if (UAbilitySystemComponent* ASC = AbilitySystem.Get())
	{
		for (const FGameplayAttribute& Attribute : BWStatusPanel::GetWatchedAttributes())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(Attribute).RemoveAll(this);
		}
	}
	AbilitySystem.Reset();
}

// Between Worlds — Etapa 1.6 (Atributos e Gameplay Effects)

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "BWResourceSet.generated.h"

#ifndef BW_ATTRIBUTE_ACCESSORS
// Gera GetXAttribute(), GetX(), SetX() e InitX() para cada atributo.
#define BW_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
#endif

/**
 * Recursos de habilidade: Mana e Stamina (GAS_ARCHITECTURE §5).
 * Gastos e recuperação acontecem só por Gameplay Effects (custo de habilidade,
 * regeneração). Valores sempre limitados entre 0 e o máximo.
 * Vive no BWPlayerState (junto do ASC).
 */
UCLASS()
class BETWEENWORLDS_API UBWResourceSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UBWResourceSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "BW|Resources")
	FGameplayAttributeData Mana;
	BW_ATTRIBUTE_ACCESSORS(UBWResourceSet, Mana)

		UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "BW|Resources")
	FGameplayAttributeData MaxMana;
	BW_ATTRIBUTE_ACCESSORS(UBWResourceSet, MaxMana)

		UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "BW|Resources")
	FGameplayAttributeData Stamina;
	BW_ATTRIBUTE_ACCESSORS(UBWResourceSet, Stamina)

		UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "BW|Resources")
	FGameplayAttributeData MaxStamina;
	BW_ATTRIBUTE_ACCESSORS(UBWResourceSet, MaxStamina)

protected:
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);
};
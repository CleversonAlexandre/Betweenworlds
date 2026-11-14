// Between Worlds — Etapa 2.7 (atributos de combate)

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "BWCombatSet.generated.h"

#ifndef BW_ATTRIBUTE_ACCESSORS
#define BW_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
#endif

/**
 * Atributos de combate (GAS_ARCHITECTURE §5, "CombatSet").
 *
 * Por enquanto só DamageMultiplier (1 = normal; 1.25 = +25% de dano causado), lido pelo
 * BWDamageExecution do lado de quem ataca. Buffs como o Grito de Guerra o alteram por
 * Gameplay Effect com duração. Os demais atributos derivados (ataque, defesas, crítico…)
 * entram aqui na Etapa 3, quando as fórmulas forem aprovadas.
 *
 * Todo personagem que causa dano tem este set (jogadores no PlayerState, inimigos no ator).
 */
UCLASS()
class BETWEENWORLDS_API UBWCombatSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UBWCombatSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DamageMultiplier, Category = "BW|Combat")
	FGameplayAttributeData DamageMultiplier;
	BW_ATTRIBUTE_ACCESSORS(UBWCombatSet, DamageMultiplier)

protected:
	UFUNCTION()
	void OnRep_DamageMultiplier(const FGameplayAttributeData& OldValue);
};

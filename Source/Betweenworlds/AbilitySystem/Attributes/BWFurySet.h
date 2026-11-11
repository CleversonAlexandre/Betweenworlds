// Between Worlds — Etapa 2.2 (Fúria e dano)

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "BWFurySet.generated.h"

#ifndef BW_ATTRIBUTE_ACCESSORS
// Gera GetXAttribute(), GetX(), SetX() e InitX() para cada atributo.
#define BW_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
#endif

/**
 * Fúria, o recurso do Guerreiro (GDD §5.3).
 *
 * Set próprio, e não dentro do ResourceSet: cada classe recebe só os sets do seu
 * recurso (GAS_ARCHITECTURE §5). O BWPlayerState cria este set em tempo de jogo
 * a partir da lista ClassAttributeSets do BP_BWPlayerState.
 *
 * Ganho, gasto e decaimento acontecem só por Gameplay Effects.
 */
UCLASS()
class BETWEENWORLDS_API UBWFurySet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UBWFurySet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Fury, Category = "BW|Fury")
	FGameplayAttributeData Fury;
	BW_ATTRIBUTE_ACCESSORS(UBWFurySet, Fury)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxFury, Category = "BW|Fury")
	FGameplayAttributeData MaxFury;
	BW_ATTRIBUTE_ACCESSORS(UBWFurySet, MaxFury)

protected:
	UFUNCTION()
	void OnRep_Fury(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxFury(const FGameplayAttributeData& OldValue);
};

// Between Worlds — Etapa 1.6 (Atributos e Gameplay Effects)

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "BWHealthSet.generated.h"

#ifndef BW_ATTRIBUTE_ACCESSORS
// Gera GetXAttribute(), GetX(), SetX() e InitX() para cada atributo.
#define BW_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
#endif

/**
 * Vida do personagem (GAS_ARCHITECTURE §5).
 *
 * Contrato de dano: NINGUÉM altera Health diretamente para causar dano. O dano
 * entra pelo meta-atributo IncomingDamage (via Gameplay Effect, no servidor) e é
 * convertido aqui em perda de Health, já limitada entre 0 e MaxHealth.
 *
 * Vive no BWPlayerState (junto do ASC). Morte ainda não existe: ao chegar a 0,
 * só registra no log (morte/respawn = Etapa 2).
 */
UCLASS()
class BETWEENWORLDS_API UBWHealthSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UBWHealthSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "BW|Health")
	FGameplayAttributeData Health;
	BW_ATTRIBUTE_ACCESSORS(UBWHealthSet, Health)

		UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "BW|Health")
	FGameplayAttributeData MaxHealth;
	BW_ATTRIBUTE_ACCESSORS(UBWHealthSet, MaxHealth)

		/** Meta-atributo: só existe no servidor durante a aplicação do efeito. Não replica. */
		UPROPERTY(BlueprintReadOnly, Category = "BW|Health")
	FGameplayAttributeData IncomingDamage;
	BW_ATTRIBUTE_ACCESSORS(UBWHealthSet, IncomingDamage)

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
};
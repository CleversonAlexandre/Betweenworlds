// Between Worlds — Etapa 1.4-b (InputTags: infra para as habilidades da 1.5)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h" // FGameplayTag — no .h: o UHT lê este arquivo isolado (lição do erro #3)
#include "BWInputConfig.generated.h"

class UInputAction;

/**
 * Par "botão → tag" (GAS_ARCHITECTURE §7): é a ligação input → habilidade que a
 * 1.5 consulta. A MESMA Input Action vale para teclado e controle — as duas
 * entradas são mapeadas no IMC_DirectControl para a mesma IA.
 */
USTRUCT()
struct FBWInputAction
{
	GENERATED_BODY()

	/** Ação de input do slot (IA_Ability1, IA_Ability2...). */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> InputAction = nullptr;

	/** Tag do slot que a ação dispara (InputTag.Ability.1...), definida no BWGameplayTags.h. */
	UPROPERTY(EditDefaultsOnly, meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/**
 * Config de input com tags (Etapa 1.4-b). Um único asset (DA_BWInputConfig)
 * centraliza o roteamento botão → InputTag; o BWPlayerController faz o bind de
 * cada entrada e leva a tag no payload. Decisão B1 da 1.4-b: os inputs NATIVOS
 * (WASD, clique-para-mover, zoom) NÃO entram aqui — este asset é só das habilidades.
 * Na 1.5 o AbilitySystemComponent (PlayerState) recebe a tag e ativa a habilidade.
 */
UCLASS(BlueprintType, Const)
class BETWEENWORLDS_API UBWInputConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Devolve a Input Action da tag pedida (ou null, com warning no log). */
	UInputAction* FindInputActionForTag(const FGameplayTag& Tag) const;

	/** Lista de ações com tag (botão → slot de habilidade). */
	UPROPERTY(EditDefaultsOnly, meta = (TitleProperty = "InputTag"))
	TArray<FBWInputAction> TaggedInputActions;
};

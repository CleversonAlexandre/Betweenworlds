// Between Worlds — Etapa 1.5 (GAS fundação: ASC no PlayerState + primeira habilidade)

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h" // FGameplayTag da struct de grant — no .h: o UHT lê este arquivo isolado (lição do erro #3)
#include "Templates/SubclassOf.h" // TSubclassOf da struct de grant
#include "BWPlayerState.generated.h"

class UBWAbilitySystemComponent;
class UGameplayAbility;

/**
 * Concessão de habilidade: a classe + o slot (InputTag) que a aciona.
 * GAS_ARCHITECTURE §7: InputTag.* é a ligação input → habilidade (tags nativas
 * do BWGameplayTags.h — nunca digitadas à mão).
 */
USTRUCT()
struct FBWAbilityGrant
{
	GENERATED_BODY()

	/** Habilidade concedida (ex.: GA_Dash, criada no editor). */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayAbility> AbilityClass = nullptr;

	/** Slot que aciona a habilidade (InputTag.Ability.1 / .2). */
	UPROPERTY(EditDefaultsOnly, meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/**
 * PlayerState do jogador. Etapa 1.5: aqui vive o AbilitySystemComponent
 * (TECHNICAL_DESIGN §4) — sobrevive a respawn e replica ao cliente dono.
 *
 * Replication mode MIXED (padrão para player-controlled): a lista de habilidades
 * concedidas replica ao CLIENTE DONO (é o que permite predição de input) e os
 * GameplayEffects ficam leves para os outros jogadores.
 *
 * O BWPlayerCharacter (avatar) liga este ASC a si mesmo em PossessedBy (servidor)
 * e OnRep_PlayerState (cliente) e pede o grant no servidor. Configurar
 * DefaultAbilities no BP_BWPlayerState.
 */
UCLASS()
class BETWEENWORLDS_API ABWPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABWPlayerState();

	/** IAbilitySystemInterface: o ASC vive aqui mesmo no PlayerState. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** Getter tipado para o código do projeto. */
	UBWAbilitySystemComponent* GetBWAbilitySystemComponent() const { return AbilitySystemComponent; }

	/**
	 * Concede DefaultAbilities ao ASC. SÓ servidor (chamado pelo BWPlayerCharacter
	 * ao ser possuído); cada PlayerState concede UMA vez (guard). No cliente a lista
	 * chega replicada (Mixed) — não se concede nada lá.
	 */
	void GrantDefaultAbilities();

protected:
	/** AbilitySystemComponent do jogador (Etapa 1.5). */
	UPROPERTY(VisibleAnywhere, Category = "BW|Abilities")
	TObjectPtr<UBWAbilitySystemComponent> AbilitySystemComponent;

	/** Habilidades concedidas no início da sessão (servidor). Configurar no BP_BWPlayerState. */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Abilities")
	TArray<FBWAbilityGrant> DefaultAbilities;

private:
	/** Guard do grant: uma única vez por PlayerState (respawn não reconcede). */
	bool bAbilitiesGranted = false;
};

// Between Worlds — Etapa 1.5 (GAS fundação) / 1.6 (atributos e Gameplay Effects) / 2.2 (sets por classe) / 2.3 (respawn) / 2.7 (CombatSet)

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTagContainer.h" // FGameplayTag da struct de grant — no .h: o UHT lê este arquivo isolado
#include "Templates/SubclassOf.h" // TSubclassOf da struct de grant
#include "BWPlayerState.generated.h"

class UBWAbilitySystemComponent;
class UBWHealthSet;
class UBWResourceSet;
class UBWCombatSet;
class UAttributeSet;
class UGameplayAbility;
class UGameplayEffect;

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

	/** Slot que aciona a habilidade (InputTag.Ability.1 / .2). Vazio = passiva (ativada por evento, sem tecla). */
	UPROPERTY(EditDefaultsOnly, meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/**
 * PlayerState do jogador. Aqui vivem o AbilitySystemComponent e os AttributeSets
 * (TECHNICAL_DESIGN §4): sobrevivem a respawn e replicam ao grupo.
 *
 * Replication mode MIXED: a lista de habilidades e os efeitos completos replicam
 * ao CLIENTE DONO; atributos replicam a todos (por propriedade).
 *
 * O BWPlayerCharacter (avatar) liga este ASC a si mesmo em PossessedBy (servidor)
 * e OnRep_PlayerState (cliente) e pede o grant no servidor. Configurar
 * DefaultAbilities e DefaultEffects no BP_BWPlayerState.
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

	UBWHealthSet* GetHealthSet() const { return HealthSet; }
	UBWResourceSet* GetResourceSet() const { return ResourceSet; }

	/**
	 * Cria ClassAttributeSets, aplica DefaultEffects (valores iniciais, regeneração)
	 * e concede DefaultAbilities, nessa ordem.
	 * SÓ servidor (chamado pelo BWPlayerCharacter ao ser possuído); cada PlayerState
	 * faz isso UMA vez (guard) — respawn não reconcede nem reinicia atributos.
	 */
	void GrantDefaultAbilities();

	/** Etapa 2.3 (servidor): aplica RespawnEffects (vida e recursos de volta ao início). */
	void ApplyRespawnEffects();

protected:
	/** AbilitySystemComponent do jogador (Etapa 1.5). */
	UPROPERTY(VisibleAnywhere, Category = "BW|Abilities")
	TObjectPtr<UBWAbilitySystemComponent> AbilitySystemComponent;

	/** Vida (Etapa 1.6). Subobjeto do PlayerState: o ASC o registra sozinho. */
	UPROPERTY()
	TObjectPtr<UBWHealthSet> HealthSet;

	/** Mana e Stamina (Etapa 1.6). */
	UPROPERTY()
	TObjectPtr<UBWResourceSet> ResourceSet;

	/** Atributos de combate (Etapa 2.7: multiplicador de dano). */
	UPROPERTY()
	TObjectPtr<UBWCombatSet> CombatSet;

	/**
	 * Sets de atributos próprios da classe (ex.: BWFurySet no Guerreiro), criados no servidor
	 * no início da sessão, ANTES dos DefaultEffects. Vida, Mana e Stamina já existem sempre.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Abilities")
	TArray<TSubclassOf<UAttributeSet>> ClassAttributeSets;

	/** Habilidades concedidas no início da sessão (servidor). Configurar no BP_BWPlayerState. */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Abilities")
	TArray<FBWAbilityGrant> DefaultAbilities;

	/**
	 * Efeitos aplicados no início da sessão (servidor), em ordem.
	 * Ex.: GE_InitAttributes (valores iniciais) e GE_StaminaRegen (regeneração).
	 */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Abilities")
	TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

	/**
	 * Efeitos aplicados a cada respawn (servidor), em ordem. Só os que RESTAURAM valores
	 * (ex.: GE_InitAttributes, GE_Warrior_InitFury). Não repetir efeitos periódicos/infinitos
	 * (regeneração, decaimento): esses continuam ativos no ASC, que sobrevive à morte.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "BW|Abilities")
	TArray<TSubclassOf<UGameplayEffect>> RespawnEffects;

private:
	/** Aplica uma lista de efeitos neste ASC (servidor). */
	void ApplyEffectsToSelf(const TArray<TSubclassOf<UGameplayEffect>>& Effects);

	/** Guard do grant: uma única vez por PlayerState (respawn não reconcede). */
	bool bAbilitiesGranted = false;
};

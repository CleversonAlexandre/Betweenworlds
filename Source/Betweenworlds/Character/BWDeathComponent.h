// Between Worlds — Etapa 2.3 (Morte e respawn)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BWDeathComponent.generated.h"

class UAbilitySystemComponent;
struct FGameplayEventData;

UENUM(BlueprintType)
enum class EBWDeathState : uint8
{
	Alive,
	Dead
};

DECLARE_MULTICAST_DELEGATE_OneParam(FBWOnDeathStarted, AActor* /*DeadActor*/);

/**
 * Morte de qualquer personagem (jogadores agora, inimigos na 2.4).
 *
 * Fluxo:
 *  1. Servidor: o BWHealthSet dispara Event.Death quando a vida chega a 0.
 *  2. Este componente (servidor) recebe o evento, muda DeathState para Dead e avisa
 *     quem estiver ouvindo OnDeathStarted (jogador: pede respawn ao GameMode).
 *  3. Todas as máquinas (servidor direto, clientes pelo OnRep) aplicam o mesmo efeito
 *     local: tag State.Dead no ASC, movimento desligado, cápsula sem colisão e animação
 *     de morte (montage Montage.Death do visual; sem montage, ragdoll).
 *
 * O estado replicado é do componente, e não uma tag do GAS, porque o ASC do jogador
 * vive no PlayerState e sobrevive ao respawn: a morte pertence a ESTE corpo.
 * A tag State.Dead é só o reflexo local desse estado, para o GAS bloquear ações.
 */
UCLASS(ClassGroup = (BW), meta = (BlueprintSpawnableComponent, DisplayName = "BW Death"))
class BETWEENWORLDS_API UBWDeathComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBWDeathComponent();

	/**
	 * Liga o componente ao ASC do personagem. Chamar em servidor e clientes sempre que
	 * o ASC for (re)ligado ao avatar. Sincroniza a tag State.Dead com o estado deste corpo.
	 */
	void InitializeWithAbilitySystem(UAbilitySystemComponent* InAbilitySystem);

	UFUNCTION(BlueprintPure, Category = "BW|Death")
	bool IsDead() const { return DeathState == EBWDeathState::Dead; }

	/** Servidor: disparado uma vez quando este corpo morre. */
	FBWOnDeathStarted OnDeathStarted;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(ReplicatedUsing = OnRep_DeathState)
	EBWDeathState DeathState = EBWDeathState::Alive;

	UFUNCTION()
	void OnRep_DeathState();

	/** Servidor: resposta ao Event.Death vindo do ASC. */
	void HandleDeathEvent(const FGameplayEventData* Payload);

	/** Servidor: inicia a morte deste corpo (uma vez). */
	void StartDeath();

	/** Todas as máquinas: efeitos locais da morte. */
	void ApplyDeathLocally();

	/** Mantém a tag State.Dead do ASC igual ao estado deste corpo. */
	void SyncDeadTag();

	void UnbindFromAbilitySystem();

	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystem;
	FDelegateHandle DeathEventHandle;
	bool bDeathAppliedLocally = false;
};

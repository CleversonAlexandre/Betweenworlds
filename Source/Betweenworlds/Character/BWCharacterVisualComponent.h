// Between Worlds — Etapa 2.1 (Visual por Data Asset)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "BWCharacterVisualComponent.generated.h"

class UBWCharacterVisualData;
class UAnimMontage;

/**
 * Aplica um UBWCharacterVisualData na malha do personagem e responde "qual montage usar para a tag X".
 *
 * Componente (e não código no personagem) para servir igualmente a jogadores e inimigos
 * sem mexer nas classes de personagem existentes.
 *
 * Rede: o asset é um valor padrão do Blueprint, igual em servidor e clientes, então cada máquina
 * aplica o mesmo visual sozinha; nada é replicado. Troca de visual em tempo de jogo
 * (equipamentos) vai exigir replicação e fica para quando existir.
 */
UCLASS(ClassGroup = (BW), meta = (BlueprintSpawnableComponent, DisplayName = "BW Character Visual"))
class BETWEENWORLDS_API UBWCharacterVisualComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBWCharacterVisualComponent();

	/** Visual a aplicar. Vazio = o personagem mantém a malha e o AnimBP definidos no Blueprint. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UBWCharacterVisualData> VisualData;

	/** Aplica malha, AnimBP e transformação. Chamado automaticamente ao registrar o componente. */
	UFUNCTION(BlueprintCallable, Category = "Visual")
	void ApplyVisual();

	/** Montage da tag no visual atual (nullptr se não houver). Usado pelas habilidades. */
	UFUNCTION(BlueprintCallable, Category = "Visual", meta = (GameplayTagFilter = "Montage"))
	UAnimMontage* GetMontage(FGameplayTag MontageTag) const;

	/** Atalho para habilidades: acha o componente no ator (ex.: o avatar da habilidade). */
	UFUNCTION(BlueprintPure, Category = "Visual")
	static UBWCharacterVisualComponent* FindVisualComponent(const AActor* Actor);

protected:
	virtual void OnRegister() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

// Between Worlds — Etapa 2.1 (Visual por Data Asset)

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "BWCharacterVisualData.generated.h"

class USkeletalMesh;
class UAnimInstance;
class UAnimMontage;

/**
 * Tudo o que define a APARÊNCIA de um personagem: mesh, Animation Blueprint e montages.
 *
 * Regra do projeto (GDD §13): gameplay nunca referencia mesh/animação diretamente.
 * Trocar o Greystone (Paragon, provisório) por um personagem próprio = trocar este asset.
 *
 * Todas as referências são SOFT: se a pasta do pack for apagada, o projeto continua
 * abrindo e compilando; o personagem só mantém o visual padrão do Blueprint e o log avisa.
 */
UCLASS(BlueprintType, Const)
class BETWEENWORLDS_API UBWCharacterVisualData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Malha do personagem. Vazio = mantém a malha definida no Blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	/** Animation Blueprint compatível com o esqueleto da malha acima. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TSoftClassPtr<UAnimInstance> AnimClass;

	/** Malhas de packs diferentes costumam precisar de posição/rotação próprias dentro da cápsula. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual|Transform")
	bool bOverrideMeshTransform = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual|Transform", meta = (EditCondition = "bOverrideMeshTransform"))
	FVector MeshRelativeLocation = FVector(0.0, 0.0, -90.0);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual|Transform", meta = (EditCondition = "bOverrideMeshTransform"))
	FRotator MeshRelativeRotation = FRotator(0.0, -90.0, 0.0);

	/** Montages por tag (Montage.Attack.Primary.1, Montage.Death…). Ver BWMontageTags.h. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual|Montages", meta = (Categories = "Montage", ForceInlineRow))
	TMap<FGameplayTag, TSoftObjectPtr<UAnimMontage>> Montages;

	/**
	 * Carrega e devolve o montage da tag. nullptr se não houver entrada ou o asset não existir.
	 * Carregamento síncrono: aceitável no protótipo (dívida técnica: pré-carregar de forma assíncrona).
	 */
	UAnimMontage* LoadMontage(const FGameplayTag& MontageTag) const;
};

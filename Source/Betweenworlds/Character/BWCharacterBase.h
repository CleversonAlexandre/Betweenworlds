// Between Worlds — Etapa 1.2 (Fundação: rede)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BWCharacterBase.generated.h"

/**
 * Base comum de jogadores e inimigos: colisão, movimento e (a partir da Etapa 1.5)
 * a interface com o Gameplay Ability System.
 *
 * Abstract: nunca é usado diretamente.
 */
UCLASS(Abstract)
class BETWEENWORLDS_API ABWCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ABWCharacterBase();
};

// Between Worlds — Etapa 1.4-b (InputTags: infra para as habilidades da 1.5)

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * GameplayTags NATIVAS do projeto (GAS_ARCHITECTURE §7): declaradas aqui em C++,
 * nunca digitadas à mão dentro de um asset. Ao carregar o módulo, elas aparecem
 * sozinhas nos seletores de tag do editor (ex.: ao montar o DA_BWInputConfig).
 *
 * Convenção: a tag "InputTag.Ability.1" vira o identificador C++
 * "InputTag_Ability_1" (pontos → underscores), padrão do Lyra.
 *
 * Sem macro de export (BETWEENWORLDS_API): o projeto é de módulo único — as
 * tags são usadas dentro do próprio módulo (controller hoje, ASC na 1.5).
 */

/** Slot de habilidade 1 (teclado: 1 / controle: Gamepad Face Button Left). */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Ability_1);

/** Slot de habilidade 2 (teclado: 2 / controle: Gamepad Face Button Top). */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Ability_2);

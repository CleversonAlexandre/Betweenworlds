// Between Worlds — Etapa 1.4-b (InputTags: infra para as habilidades da 1.5) / 2.5 (ataque principal)

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

/** Etapa 2.5: ataque principal (mouse: clique em inimigo; controle: botão de ataque). */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Attack_Primary);

/**
 * Etapa 2.5: habilidade com esta Asset Tag continua sendo reativada enquanto o botão
 * do slot estiver segurado (ex.: Corte em combo). Sem ela, segurar não repete (ex.: Esquiva).
 */
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Input_RepeatWhileHeld);

// Etapa 1.6 — tag concedida pelo GE_Cooldown_Dash enquanto o dash está em recarga.
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cooldown_Ability_Dash);

// Between Worlds — Etapa 2.2 (Fúria e dano)

#pragma once

#include "NativeGameplayTags.h"

/**
 * Tags de combate (eventos, estados e SetByCaller).
 * Arquivo separado de BWGameplayTags para não reescrever aquele arquivo.
 */

/** Disparado pelo BWHealthSet no servidor sempre que o dono perde vida. EventMagnitude = dano. */
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Damage_Received);

/** Presente enquanto o personagem está em combate (dado por GE_InCombat, com duração). */
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_InCombat);

/** SetByCaller: dano base de um efeito que usa o UBWDamageExecution. */
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Data_Damage);

/** SetByCaller: quantidade de Fúria ganha (GE_FuryGain). */
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Data_Fury);

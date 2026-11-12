// Between Worlds — Etapa 2.2 (Fúria e dano) / 2.3 (morte) / 2.4 (acerto corpo a corpo)

#pragma once

#include "NativeGameplayTags.h"

/**
 * Tags de combate (eventos, estados e SetByCaller).
 * Arquivo separado de BWGameplayTags para não reescrever aquele arquivo.
 */

/** Disparado pelo BWHealthSet no servidor sempre que o dono perde vida. EventMagnitude = dano. */
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Damage_Received);

/** Disparado pelo BWHealthSet no servidor quando a vida chega a 0. Target = corpo (avatar). */
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Death);

/** Personagem morto. Reflexo local do estado do BWDeathComponent: bloqueia habilidades e dano. */
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_Dead);

/** Disparado pelo AnimNotify "BW Gameplay Event" no quadro de acerto de um golpe. */
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Montage_Hit);

/** Presente enquanto o personagem está em combate (dado por GE_InCombat, com duração). */
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_State_InCombat);

/** SetByCaller: dano base de um efeito que usa o UBWDamageExecution. */
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Data_Damage);

/** SetByCaller: quantidade de Fúria ganha (GE_FuryGain). */
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Data_Fury);

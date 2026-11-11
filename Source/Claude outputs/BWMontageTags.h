// Between Worlds — Etapa 2.1 (Visual por Data Asset)

#pragma once

#include "NativeGameplayTags.h"

/**
 * Tags que identificam montages no UBWCharacterVisualData.
 * Habilidades pedem "Montage.Attack.Primary.1", nunca um asset direto,
 * para que trocar o visual de um personagem não exija mexer em habilidades.
 *
 * Arquivo separado de BWGameplayTags para não reescrever aquele arquivo.
 */

// Corte (ataque principal, combo de 3 golpes)
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Montage_Attack_Primary_1);
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Montage_Attack_Primary_2);
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Montage_Attack_Primary_3);

// Habilidades do kit da vertical slice
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Montage_Ability_CrushingBlow); // Golpe Esmagador
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Montage_Ability_WarCry);       // Grito de Guerra

// Reações, morte e entrada no mapa
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Montage_HitReact_Front);
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Montage_HitReact_Back);
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Montage_HitReact_Left);
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Montage_HitReact_Right);
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Montage_Death);
BETWEENWORLDS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Montage_Respawn);

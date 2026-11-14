// Between Worlds — Etapa 2.2 (Fúria e dano) / 2.3 (morte) / 2.4 (acerto corpo a corpo) / 2.6 (cues)

#include "AbilitySystem/Tags/BWCombatTags.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Event_Damage_Received, "Event.Damage.Received", "O dono perdeu vida (servidor). EventMagnitude = dano.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Event_Death, "Event.Death", "A vida chegou a 0 (servidor). Target = corpo.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_Dead, "State.Dead", "Morto: bloqueia habilidades e dano.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Event_Montage_Hit, "Event.Montage.Hit", "Quadro de acerto de um golpe (AnimNotify).");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_GameplayCue_Warrior_CrushingBlow, "GameplayCue.Warrior.CrushingBlow", "Impacto do Golpe Esmagador.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_InCombat, "State.InCombat", "Em combate; recursos como a Fúria não decaem.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Data_Damage, "Data.Damage", "SetByCaller: dano base.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Data_Fury, "Data.Fury", "SetByCaller: Fúria ganha.");

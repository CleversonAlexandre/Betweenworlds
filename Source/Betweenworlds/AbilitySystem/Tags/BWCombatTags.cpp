// Between Worlds — Etapa 2.2 (Fúria e dano)

#include "AbilitySystem/Tags/BWCombatTags.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Event_Damage_Received, "Event.Damage.Received", "O dono perdeu vida (servidor). EventMagnitude = dano.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_State_InCombat, "State.InCombat", "Em combate; recursos como a Fúria não decaem.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Data_Damage, "Data.Damage", "SetByCaller: dano base.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Data_Fury, "Data.Fury", "SetByCaller: Fúria ganha.");

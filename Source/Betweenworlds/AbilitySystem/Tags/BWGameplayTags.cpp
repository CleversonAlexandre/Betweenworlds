// Between Worlds — Etapa 1.4-b (InputTags: infra para as habilidades da 1.5)

#include "AbilitySystem/Tags/BWGameplayTags.h"

// O texto entre aspas é a tag EXATA gravada no DA_BWInputConfig e vista nos logs.
// UE_DEFINE_GAMEPLAY_TAG_COMMENT deixa o comentário visível no seletor do editor.
UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Ability_1,
	"InputTag.Ability.1",
	"Slot de habilidade 1 (teclado: 1 / controle: Gamepad Face Button Left)");

UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Ability_2,
	"InputTag.Ability.2",
	"Slot de habilidade 2 (teclado: 2 / controle: Gamepad Face Button Top)");

// Etapa 2.5
UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Attack_Primary, "InputTag.Attack.Primary",
	"Ataque principal (mouse: clique em inimigo / controle: botão de ataque)");
// Etapa 2.6
UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Dodge, "InputTag.Dodge",
	"Esquiva universal (teclado: Espaço / controle: Gamepad Face Button Right)");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Ability_Input_RepeatWhileHeld, "Ability.Input.RepeatWhileHeld",
	"Habilidade reativada enquanto o botão do slot estiver segurado (combo).");

// Etapa 1.6
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Cooldown_Ability_Dash, "Cooldown.Ability.Dash", "Dash em recarga (concedida pelo GE_Cooldown_Dash).");

// Etapa 2.7
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Cooldown_Ability_WarCry, "Cooldown.Ability.WarCry", "Grito de Guerra em recarga (concedida pelo GE_Cooldown_WarCry).");

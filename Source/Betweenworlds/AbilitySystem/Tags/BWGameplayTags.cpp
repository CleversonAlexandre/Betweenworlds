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

// Etapa 1.6
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Cooldown_Ability_Dash, "Cooldown.Ability.Dash", "Dash em recarga (concedida pelo GE_Cooldown_Dash).");

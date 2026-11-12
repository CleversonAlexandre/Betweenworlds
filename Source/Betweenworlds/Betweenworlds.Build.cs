// Between Worlds — arquivo de build do módulo Betweenworlds
// Etapa 1.4-a: adicionado "NavigationSystem" (navmesh, caminhos para o clique-para-mover).
// Etapa 2.4: "AIModule", "StateTreeModule" e "GameplayStateTreeModule" (IA dos inimigos).
// O clique-para-mover do jogador continua sem AIModule (BWPathFollowComponent).

using UnrealBuildTool;

public class Betweenworlds : ModuleRules
{
	public Betweenworlds(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// OBRIGATÓRIO: permite includes relativos à raiz do módulo,
		// ex.: #include "Character/BWCharacterBase.h". Não remover.
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG",
			"NavigationSystem",

			// Etapa 2.4: IA dos inimigos (AIController, StateTree).
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",

			// GAS: público porque headers do projeto expõem tipos do GAS.
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
	}
}

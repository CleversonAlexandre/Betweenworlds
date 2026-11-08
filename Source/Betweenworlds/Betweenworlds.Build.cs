// Between Worlds — regras de build do módulo principal (módulo "Betweenworlds").
// Etapa 1 (Fundação): apenas as dependências necessárias para rede, GAS e Enhanced Input.

using UnrealBuildTool;

public class Betweenworlds : ModuleRules
{
	public Betweenworlds(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Permite includes relativos à raiz do módulo, ex.: #include "Core/BWPlayerController.h"
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",

			// GAS: público porque headers do projeto expõem tipos do GAS.
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}

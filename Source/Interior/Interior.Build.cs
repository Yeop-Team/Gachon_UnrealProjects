// Copyright Yeop. All Rights Reserved.

using UnrealBuildTool;

public class Interior : ModuleRules
{
	public Interior(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore",
			"Niagara",
			"GameplayTags"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AIModule"
		});

		PublicIncludePaths.AddRange(new string[] { "Interior/Public" });
		PrivateIncludePaths.AddRange(new string[] { "Interior/Private" });
	}
}

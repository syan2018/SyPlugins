// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SyCombat : ModuleRules
{
	public SyCombat(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"StructUtils",

				// Sy
				"SyCore",
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Engine",
			});
	}
}


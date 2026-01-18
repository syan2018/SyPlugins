// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SyCombatLyraAdapter : ModuleRules
{
	public SyCombatLyraAdapter(ReadOnlyTargetRules Target) : base(Target)
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
				"SyCombat",
				"SyGASBridge",

				// GAS
				"GameplayAbilities",
				"GameplayTasks",
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Engine",
			});
	}
}


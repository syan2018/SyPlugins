// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SyGASBridge : ModuleRules
{
	public SyGASBridge(ReadOnlyTargetRules Target) : base(Target)
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


// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class SyGameplay : ModuleRules
{
	public SyGameplay(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
				Path.Combine(ModuleDirectory, "Public"), // Base Public path
				Path.Combine(ModuleDirectory, "Public/Metadatas"), // Metadata Public path
				// Include path for SyStateCore if needed for FSyListParameterBase, etc.
				// Assuming SyStateCore is in the same Plugins folder
				// You might need to adjust this path based on your project structure
				// System.IO.Path.Combine(PluginDirectory, "../SyStateCore/Source/SyStateCore/Public"),
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
				Path.Combine(ModuleDirectory, "Private"), // Base Private path
				Path.Combine(ModuleDirectory, "Private/Metadatas"), // Metadata Private path
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"SyCore",
				"SyEntity",
				"SyStateSystem",
				"TagMetadata",
				"AIModule",
				"GameplayTags",
				"UMG",
				"StructUtils",
				"StateTreeModule",
				"Flow",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}

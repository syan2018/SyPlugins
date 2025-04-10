// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SyStateCoreEditor : ModuleRules
{
	public SyStateCoreEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
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
				"InputCore", // Needed for some Editor customizations
                "UnrealEd", // Needed for Editor specific functionalities
                "PropertyEditor", // Needed for IPropertyTypeCustomization
                "GameplayTags", // Needed for FGameplayTag
                "GameplayTagsEditor", // Potentially needed for Tag Editor integration
                "TagMetadata", // Needed for TagMetadata plugin access (UDS_TagMetadata)
                "SyStateCore", // Dependency on the Runtime module
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
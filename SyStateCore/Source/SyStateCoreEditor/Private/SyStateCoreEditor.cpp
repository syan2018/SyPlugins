// Copyright Epic Games, Inc. All Rights Reserved.

#include "SyStateCoreEditor.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"
#include "StateParameterTypes.h" // Need FSyStateParameterSet
// Include the customization class header
#include "FSyStateParameterSetCustomization.h"

#define LOCTEXT_NAMESPACE "FSyStateCoreEditorModule"

void FSyStateCoreEditorModule::StartupModule()
{
	// Register property customizations
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// Register the customization for FSyStateParameterSet
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FSyStateParameterSet::StaticStruct()->GetFName(), 
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSyStateParameterSetCustomization::MakeInstance)
	);

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FSyStateCoreEditorModule::ShutdownModule()
{
	// Unregister property customizations
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		
		PropertyModule.UnregisterCustomPropertyTypeLayout(FSyStateParameterSet::StaticStruct()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSyStateCoreEditorModule, SyStateCoreEditor) 
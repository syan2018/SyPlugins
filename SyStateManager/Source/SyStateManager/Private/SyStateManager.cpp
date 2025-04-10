// Copyright Epic Games, Inc. All Rights Reserved.

#include "SyStateManager.h"

#define LOCTEXT_NAMESPACE "FSyStateManagerModule"

void FSyStateManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	// UE_LOG(LogTemp, Warning, TEXT("SyStateManager module has started!"));
}

void FSyStateManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	// UE_LOG(LogTemp, Warning, TEXT("SyStateManager module has shut down!"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSyStateManagerModule, SyStateManager)
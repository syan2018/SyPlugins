// Copyright Epic Games, Inc. All Rights Reserved.

#include "SyStateSystem.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FSyStateSystemModule"

void FSyStateSystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UE_LOG(LogTemp, Log, TEXT("SyStateSystem module has started - Unified State Management System"));
}

void FSyStateSystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UE_LOG(LogTemp, Log, TEXT("SyStateSystem module has shut down"));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSyStateSystemModule, SyStateSystem)

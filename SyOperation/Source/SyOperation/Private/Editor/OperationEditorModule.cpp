// Copyright Epic Games, Inc. All Rights Reserved.

#include "Editor/OperationEditorModule.h"
#include "Editor/OperationDetailsCustomization.h"
#include "PropertyEditorModule.h"
#include "OperationTypes.h"

#define LOCTEXT_NAMESPACE "FSyOperationEditorModule"

void FSyOperationEditorModule::StartupModule()
{
    // 注册自定义详情视图
    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    
    // 注册FSyOperation的自定义详情视图
    PropertyModule.RegisterCustomClassLayout(
        FName(FSyOperation::StaticStruct()->GetName()),
        FOnGetDetailCustomizationInstance::CreateStatic(&FSyOperationDetailsCustomization::MakeInstance)
    );
    
    PropertyModule.NotifyCustomizationModuleChanged();
}

void FSyOperationEditorModule::ShutdownModule()
{
    // 取消注册自定义详情视图
    if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
    {
        FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        
        // 取消注册FSyOperation的自定义详情视图
        PropertyModule.UnregisterCustomClassLayout(FName(FSyOperation::StaticStruct()->GetName()));
        
        PropertyModule.NotifyCustomizationModuleChanged();
    }
}

#undef LOCTEXT_NAMESPACE 
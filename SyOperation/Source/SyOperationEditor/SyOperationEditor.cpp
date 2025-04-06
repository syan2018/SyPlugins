// Copyright Epic Games, Inc. All Rights Reserved.

#include "SyOperationEditor.h"
#include "Modules/ModuleManager.h"
#include "SyOperation/Public/Editor/OperationEditorModule.h"

#define LOCTEXT_NAMESPACE "FSyOperationEditorModule"

// 创建一个全局的FSyOperationEditorModule实例
static FSyOperationEditorModule* GOperationEditorModule = nullptr;

void FSyOperationEditorModuleImpl::StartupModule()
{
    // 创建FSyOperationEditorModule的实例
    GOperationEditorModule = new FSyOperationEditorModule();
    
    // 调用FSyOperationEditorModule的StartupModule方法
    GOperationEditorModule->StartupModule();
}

void FSyOperationEditorModuleImpl::ShutdownModule()
{
    // 调用FSyOperationEditorModule的ShutdownModule方法
    if (GOperationEditorModule)
    {
        GOperationEditorModule->ShutdownModule();
        
        // 删除实例
        delete GOperationEditorModule;
        GOperationEditorModule = nullptr;
    }
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FSyOperationEditorModuleImpl, SyOperationEditor) 
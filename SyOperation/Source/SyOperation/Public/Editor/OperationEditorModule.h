// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * FSyOperationEditorModule - 操作编辑器模块
 * 
 * 用于注册编辑器自定义类和工具
 */
class SYOPERATION_API FSyOperationEditorModule : public IModuleInterface
{
public:
    /** IModuleInterface实现 */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
}; 
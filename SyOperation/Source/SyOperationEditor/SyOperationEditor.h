// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * FSyOperationEditorModuleImpl - 操作编辑器模块实现
 * 
 * 用于注册编辑器自定义类和工具
 */
class FSyOperationEditorModuleImpl : public IModuleInterface
{
public:
    /** IModuleInterface实现 */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
}; 
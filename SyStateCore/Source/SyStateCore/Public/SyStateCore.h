// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// Include the new type headers for easier access from outside the module
#include "StateContainerTypes.h"
#include "StateParameterTypes.h"
#include "StateMetadataTypes.h"

/**
 * SyStateCore模块 - 定义状态管理系统的核心数据结构和类型
 * 
 * 本模块提供：
 * 1. 状态容器结构 (FSyStateCategories, FSyStateMetadatas)
 * 2. 状态参数结构 (FSyStateParams, FSyStateParameterSet)
 * 3. 状态元数据类 (USyStateMetadataBase, USyTagStateMetadata) - 通过TagMetadata插件存储状态数据
 */
class FSyStateCoreModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

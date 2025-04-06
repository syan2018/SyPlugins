// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * SyStateCore模块 - 定义状态管理系统的核心数据结构和类型
 * 
 * 本模块提供：
 * 1. FSyEntityState结构体，用于存储实体的状态数据
 * 2. FSyEntityInitData结构体，用于定义实体的初始状态
 * 3. 各种状态元数据类，用于通过TagMetadata插件存储状态数据
 */
class FSyStateCoreModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

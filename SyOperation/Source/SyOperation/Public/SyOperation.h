// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * SyOperation模块 - 定义状态变更操作的数据结构和参数定义机制
 * 
 * 本模块提供：
 * 1. FSyOperation结构体及其子结构，用于定义状态变更操作
 * 2. USyOperationParamSchemaMetadata类，用于通过TagMetadata插件将GameplayTag与参数结构体类型关联
 * 3. 各种操作参数结构体定义
 */
class FSyOperationModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// 核心状态数据结构
#include "Core/StateContainerTypes.h"
#include "Core/StateParameterTypes.h"
#include "Core/StateMetadataTypes.h"

// 操作定义
#include "Operations/OperationTypes.h"
#include "Operations/OperationParams.h"

// 状态管理器
#include "Manager/SyStateManagerSubsystem.h"
#include "Manager/StateModificationRecord.h"
#include "Manager/SyStateManagerSaveGame.h"

/**
 * SyStateSystem模块 - 统一的状态管理系统
 * 
 * 本模块整合了原 SyStateCore、SyOperation、SyStateManager 三个模块的功能：
 * 
 * ## Core 子模块（原 SyStateCore）
 * 1. 状态容器结构 (FSyStateCategories, FSyStateMetadatas)
 * 2. 状态参数结构 (FSyStateParams, FSyStateParameterSet)
 * 3. 状态元数据类 (USyStateMetadataBase, USyTagStateMetadata)
 * 
 * ## Operations 子模块（原 SyOperation）
 * 1. FSyOperation结构体及其子结构，用于定义状态变更操作
 * 2. USyOperationParamSchemaMetadata类，用于参数类型定义
 * 3. 各种操作参数结构体定义
 * 
 * ## Manager 子模块（原 SyStateManager）
 * 1. 全局状态管理子系统 (USyStateManagerSubsystem)
 * 2. 状态修改记录管理 (FSyStateModificationRecord)
 * 3. 状态持久化支持 (USyStateManagerSaveGame)
 */
class FSyStateSystemModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

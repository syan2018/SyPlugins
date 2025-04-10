// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * SyStateManager模块接口
 * 
 * 主要职责是初始化和关闭状态管理器子系统相关的逻辑（如果需要的话）。
 */
class FSyStateManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

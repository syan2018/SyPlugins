// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SyInstancedStruct.generated.h"

/**
 * FSyBaseInstancedStruct
 * 
 * 通用配置结构体基类，用于标识和方便引用
 */
USTRUCT(BlueprintType)
struct SYCORE_API FSyBaseInstancedStruct
{
    GENERATED_BODY()
    virtual ~FSyBaseInstancedStruct() = default;
};

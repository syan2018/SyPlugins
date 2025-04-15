// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "OperationParams.generated.h"

/**
 * FInteractionSourceParams - 交互来源参数
 * 
 * 定义交互操作来源的参数
 */
USTRUCT(BlueprintType)
struct SYOPERATION_API FInteractionSourceParams
{
    GENERATED_BODY()

    /** 默认构造函数 */
    FInteractionSourceParams() = default;

    /** 从交互类型构造 */
    FInteractionSourceParams(const FGameplayTag& InInteractionType)
        : InteractionType(InInteractionType) {}

    /** 交互类型 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Interaction")
    FGameplayTag InteractionType;

    /** 交互范围 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Interaction")
    float Range = 0.0f;

    /** 交互角度 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Interaction")
    float Angle = 0.0f;

    /** 是否需要视线 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Interaction")
    bool bRequiresLineOfSight = false;
}; 
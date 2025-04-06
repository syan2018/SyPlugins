// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OperationParams.generated.h"

// 这里的东西基本都是要删掉的，我们根本不会用状态系统来处理这么细的逻辑，战斗逻辑交给GAS（（

/**
 * FDamageParams - 伤害参数
 * 
 * 定义伤害操作的基本参数
 */
USTRUCT(BlueprintType)
struct SYOPERATION_API FDamageParams
{
    GENERATED_BODY()

    /** 默认构造函数 */
    FDamageParams() = default;

    /** 从伤害值构造 */
    FDamageParams(float InDamageAmount) : DamageAmount(InDamageAmount) {}

    /** 从伤害值和伤害类型构造 */
    FDamageParams(float InDamageAmount, const FGameplayTag& InDamageType)
        : DamageAmount(InDamageAmount), DamageType(InDamageType) {}

    /** 伤害值 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Damage")
    float DamageAmount = 0.0f;

    /** 伤害类型 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Damage")
    FGameplayTag DamageType;

    /** 是否忽略护甲 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Damage")
    bool bIgnoreArmor = false;

    /** 是否造成真实伤害 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Damage")
    bool bIsTrueDamage = false;
};

/**
 * FBurningDamageParams - 燃烧伤害参数
 * 
 * 定义燃烧伤害操作的特殊参数
 */
USTRUCT(BlueprintType)
struct SYOPERATION_API FBurningDamageParams : public FDamageParams
{
    GENERATED_BODY()

    /** 默认构造函数 */
    FBurningDamageParams() = default;

    /** 从伤害值和持续时间构造 */
    FBurningDamageParams(float InDamageAmount, float InDuration)
        : FDamageParams(InDamageAmount), Duration(InDuration) {}

    /** 从伤害值、伤害类型和持续时间构造 */
    FBurningDamageParams(float InDamageAmount, const FGameplayTag& InDamageType, float InDuration)
        : FDamageParams(InDamageAmount, InDamageType), Duration(InDuration) {}

    /** 燃烧持续时间（秒） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|BurningDamage")
    float Duration = 0.0f;

    /** 每秒伤害值 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|BurningDamage")
    float DamagePerTick = 0.0f;

    /** 伤害间隔（秒） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|BurningDamage")
    float TickInterval = 1.0f;
};

/**
 * FApplyStatusParams - 应用状态参数
 * 
 * 定义应用状态效果操作的参数
 */
USTRUCT(BlueprintType)
struct SYOPERATION_API FApplyStatusParams
{
    GENERATED_BODY()

    /** 默认构造函数 */
    FApplyStatusParams() = default;

    /** 从状态类型和持续时间构造 */
    FApplyStatusParams(const FGameplayTag& InStatusType, float InDuration)
        : StatusType(InStatusType), Duration(InDuration) {}

    /** 状态类型 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Status")
    FGameplayTag StatusType;

    /** 状态持续时间（秒），0表示永久 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Status")
    float Duration = 0.0f;

    /** 状态强度 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Status")
    float Magnitude = 1.0f;

    /** 是否可叠加 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Status")
    bool bCanStack = false;

    /** 最大叠加层数，0表示无限制 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Status")
    int32 MaxStacks = 0;
};

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
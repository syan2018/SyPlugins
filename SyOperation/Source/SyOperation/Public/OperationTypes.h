// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SyCore/Public/Foundation/Utilities/SyInstancedStruct.h"
#include "SyStateCore/Public/EntityStateTypes.h"
#include "OperationTypes.generated.h"

/**
 * FSyOperationSource - 操作来源
 * 
 * 定义操作的来源，包含来源类型标签和参数
 */
USTRUCT(BlueprintType)
struct SYOPERATION_API FSyOperationSource
{
    GENERATED_BODY()

    /** 默认构造函数 */
    FSyOperationSource() = default;

    /** 从标签构造 */
    FSyOperationSource(const FGameplayTag& InSourceTypeTag) : SourceTypeTag(InSourceTypeTag) {}

    /** 从标签和参数构造 */
    FSyOperationSource(const FGameplayTag& InSourceTypeTag, const FSyInstancedStruct& InParameters)
        : SourceTypeTag(InSourceTypeTag), Parameters(InParameters) {}

    /** 来源类型标签 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Source")
    FGameplayTag SourceTypeTag;

    /** 来源参数 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Source")
    FSyInstancedStruct Parameters;
};

/**
 * FSyOperationModifier - 操作修饰器
 * 
 * 定义操作的修饰方式，包含多个状态修改
 * 每个状态修改由状态标签和对应的参数数组组成
 */
USTRUCT(BlueprintType)
struct SYOPERATION_API FSyOperationModifier
{
    GENERATED_BODY()

    /** 默认构造函数 */
    FSyOperationModifier() = default;

    /** 从修饰器标签构造 */
    FSyOperationModifier(const FGameplayTag& InModifierTag) : ModifierTag(InModifierTag) {}

    /** 从修饰器标签和状态修改映射构造 */
    FSyOperationModifier(const FGameplayTag& InModifierTag, const TMap<FGameplayTag, FSyStateParams>& InStateModifications)
        : ModifierTag(InModifierTag), StateModifications(InStateModifications) {}

    /** 修饰器类型标签 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Modifier")
    FGameplayTag ModifierTag;

    /** 状态修改映射：状态标签 -> 参数数组 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Modifier")
    TMap<FGameplayTag, FSyStateParams> StateModifications;

    /** 添加状态修改 */
    void AddStateModification(const FGameplayTag& StateTag, const FSyInstancedStruct& Parameters)
    {
        FSyStateParams& StateParams = StateModifications.FindOrAdd(StateTag);
        StateParams.AddParam(Parameters);
    }

    /** 添加多个状态修改 */
    void AddStateModifications(const FGameplayTag& StateTag, const TArray<FSyInstancedStruct>& Parameters)
    {
        FSyStateParams& StateParams = StateModifications.FindOrAdd(StateTag);
        StateParams.AddParams(Parameters);
    }

    /** 清除所有状态修改 */
    void ClearStateModifications()
    {
        StateModifications.Empty();
    }

    /** 清除特定状态标签的修改 */
    void ClearStateModificationsForTag(const FGameplayTag& StateTag)
    {
        StateModifications.Remove(StateTag);
    }
};

/**
 * FSyOperationTarget - 操作目标
 * 
 * 定义操作的目标，包含目标类型标签、参数和目标实体ID
 */
USTRUCT(BlueprintType)
struct SYOPERATION_API FSyOperationTarget
{
    GENERATED_BODY()

    /** 默认构造函数 */
    FSyOperationTarget() = default;

    /** 从标签和目标ID构造 */
    FSyOperationTarget(const FGameplayTag& InTargetTypeTag, const FGuid& InTargetEntityId)
        : TargetTypeTag(InTargetTypeTag), TargetEntityId(InTargetEntityId) {}

    /** 从标签、参数和目标ID构造 */
    FSyOperationTarget(const FGameplayTag& InTargetTypeTag, const FSyInstancedStruct& InParameters, const FGuid& InTargetEntityId)
        : TargetTypeTag(InTargetTypeTag), Parameters(InParameters), TargetEntityId(InTargetEntityId) {}

    /** 目标类型标签 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Target")
    FGameplayTag TargetTypeTag;

    /** 目标参数 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Target")
    FSyInstancedStruct Parameters;

    /** 目标实体ID */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Target")
    FGuid TargetEntityId;
};

/**
 * FSyOperation - 操作
 * 
 * 定义一个完整的状态变更意图，包含来源、修饰器和目标
 */
USTRUCT(BlueprintType)
struct SYOPERATION_API FSyOperation
{
    GENERATED_BODY()

    /** 默认构造函数 */
    FSyOperation() = default;

    /** 从来源、修饰器和目标构造 */
    FSyOperation(const FSyOperationSource& InSource, const FSyOperationModifier& InModifier, const FSyOperationTarget& InTarget)
        : Source(InSource), Modifier(InModifier), Target(InTarget)
    {
        OperationId = FGuid::NewGuid();
    }

    /** 从来源、修饰器和目标构造，并指定操作ID */
    FSyOperation(const FSyOperationSource& InSource, const FSyOperationModifier& InModifier, const FSyOperationTarget& InTarget, const FGuid& InOperationId)
        : Source(InSource), Modifier(InModifier), Target(InTarget), OperationId(InOperationId) {}

    /** 操作来源 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation")
    FSyOperationSource Source;

    /** 操作修饰器 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation")
    FSyOperationModifier Modifier;

    /** 操作目标 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation")
    FSyOperationTarget Target;

    /** 操作ID */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation")
    FGuid OperationId;
}; 
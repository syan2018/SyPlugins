// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StateParameterTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "OperationTypes.generated.h"

/**
 * FSyOperationSource - 操作来源
 * 
 * 定义操作的来源，包含来源类型标签和参数
 * 继承以实现编辑器配置和传参
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
    FSyOperationSource(const FGameplayTag& InSourceTypeTag, const FInstancedStruct& InParameters)
        : SourceTypeTag(InSourceTypeTag), Parameters(InParameters) {}

    /** 来源类型标签 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Source")
    FGameplayTag SourceTypeTag;

    /** 可选的来源实体ID */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Source", meta = (DisplayName = "Optional Source Entity ID"))
    FGuid SourceEntityId; // 使用 FGuid 而不是 TOptional<FGuid> 以简化序列化和蓝图暴露

    /** 可选的来源别名 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Source", meta = (DisplayName = "Optional Source Alias"))
    FName SourceAlias; // 使用 FName 而不是 TOptional<FName>

    /** 额外的来源参数 (用于复杂情况或非标识别) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Source", meta = (DisplayName = "Additional Source Parameters"))
    FInstancedStruct Parameters;

    // 辅助函数，检查是否有有效的ID或Alias
    bool HasValidId() const { return SourceEntityId.IsValid(); }
    bool HasValidAlias() const { return SourceAlias != NAME_None; }
};


/**
 * FSyOperationTarget - 操作目标
 * 
 * 定义操作的目标，包含目标类型标签、参数和目标实体ID
 * 继承以实现编辑器配置和传参
 */
USTRUCT(BlueprintType)
struct SYOPERATION_API FSyOperationTarget
{
    GENERATED_BODY()

    /** 默认构造函数 */
    FSyOperationTarget() = default;

    /** 从标签构造 */
    FSyOperationTarget(const FGameplayTag& InTargetTypeTag): TargetTypeTag(InTargetTypeTag) {}

    /** 从标签、参数构造 */
    FSyOperationTarget(const FGameplayTag& InTargetTypeTag, const FInstancedStruct& InParameters)
        : TargetTypeTag(InTargetTypeTag), Parameters(InParameters) {}

    /** 目标类型标签 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Target")
    FGameplayTag TargetTypeTag;

    /** 可选的目标实体ID */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Target", meta = (DisplayName = "Optional Target Entity ID"))
    FGuid TargetEntityId;

    /** 可选的目标别名 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Target", meta = (DisplayName = "Optional Target Alias"))
    FName TargetAlias;

    /** 额外的目标参数 (用于复杂情况或非标识别) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Target", meta = (DisplayName = "Additional Target Parameters"))
    FInstancedStruct Parameters;

    // 辅助函数，检查是否有有效的ID或Alias
    bool HasValidId() const { return TargetEntityId.IsValid(); }
    bool HasValidAlias() const { return TargetAlias != NAME_None; }
    
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

    /** 从修饰器标签和状态修改映射构造 */
    FSyOperationModifier(const FSyStateParameterSet& InStateModifications)
        : StateModifications(InStateModifications) {}
    
    /** 状态修改映射：状态标签 -> 参数数组 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyOperation|Modifier")
    FSyStateParameterSet StateModifications;

    /** 添加状态修改 */
    void AddStateModification(const FGameplayTag& StateTag, const FInstancedStruct& Parameters)
    {
        StateModifications.AddStateParam(StateTag, Parameters);
    }

    /** 添加多个状态修改 */
    void AddStateModifications(const FGameplayTag& StateTag, const TArray<FInstancedStruct>& Parameters)
    {
        StateModifications.AddStateParams(StateTag, Parameters);
    }

    /** 清除所有状态修改 */
    void ClearStateModifications()
    {
        StateModifications.ClearAllStateParams();
    }

    /** 清除特定状态标签的修改 */
    void ClearStateModificationsForTag(const FGameplayTag& StateTag)
    {
        StateModifications.RemoveStateParams(StateTag);
    }
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
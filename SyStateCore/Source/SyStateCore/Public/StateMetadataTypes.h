// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "O_TagMetadata.h"
#include "SyCore/Public/Foundation/Utilities/SyInstancedStruct.h"
#include "StateMetadataTypes.generated.h"

/**
 * USyStateMetadataBase - 状态元数据基类
 * 
 * 所有状态元数据类的基类，提供基本的状态属性和功能
 */
UCLASS(Abstract, Blueprintable)
class SYSTATECORE_API USyStateMetadataBase : public UO_TagMetadata
{
    GENERATED_BODY()

public:
    /** 构造函数 */
    USyStateMetadataBase();

    /** 从初始化参数创建状态元数据对象 */
    virtual void InitializeFromParams(const FSyInstancedStruct& InitParams) {}

    /** 应用状态修改 */
    virtual void ApplyModification(const FSyInstancedStruct& ModificationParams) {}

    /** 获取状态标签 */
    UFUNCTION(BlueprintPure, Category = "SyStateCore|StateMetadata")
    FGameplayTag GetStateTag() const { return StateTag; }

    /** 设置状态标签 */
    UFUNCTION(BlueprintCallable, Category = "SyStateCore|StateMetadata")
    void SetStateTag(const FGameplayTag& InStateTag) { StateTag = InStateTag; }

protected:
    /** 状态标签 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyStateCore|StateMetadata")
    FGameplayTag StateTag;
};

/**
 * USyBooleanStateMetadata - 布尔状态元数据（已删除）
 * 
 * 可以尝试使用Struct封装部分结构体来作为 Metadata，使用原生类型会无法转换（
 */

/**
 * USyTagStateMetadata - 标签状态元数据
 * 
 * 用于存储GameplayTag类型的状态数据
 */
UCLASS(Blueprintable)
class SYSTATECORE_API USyTagStateMetadata : public USyStateMetadataBase
{
    GENERATED_BODY()

public:
    /** 构造函数 */
    USyTagStateMetadata();

    /** 从初始化参数创建状态元数据对象 */
    virtual void InitializeFromParams(const FSyInstancedStruct& InitParams) override;

    /** 应用状态修改 */
    virtual void ApplyModification(const FSyInstancedStruct& ModificationParams) override;

    /** 获取标签值 */
    UFUNCTION(BlueprintPure, Category = "SyStateCore|TagState")
    FGameplayTag GetTagValue() const { return Value; }

    /** 设置标签值 */
    UFUNCTION(BlueprintCallable, Category = "SyStateCore|TagState")
    void SetTagValue(const FGameplayTag& InValue) { Value = InValue; }

protected:
    /** 标签值 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyStateCore|TagState")
    FGameplayTag Value;
}; 
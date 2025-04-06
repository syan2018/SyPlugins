// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "O_TagMetadata.h"
#include "StateMetadataTypes.generated.h"

struct FSyInstancedStruct;
/**
 * USyStateMetadataBase - 状态元数据基类
 * 
 * 所有状态元数据的基类，继承自UO_TagMetadata
 * 提供状态标签和参数处理的基本功能
 */
UCLASS(Abstract, Blueprintable)
class SYSTATECORE_API USyStateMetadataBase : public UO_TagMetadata
{
    GENERATED_BODY()

public:
    /** 构造函数 */
    USyStateMetadataBase();

    /** 从参数初始化元数据 */
    virtual void InitializeFromParams(const FSyInstancedStruct& InitParams) PURE_VIRTUAL(USyStateMetadataBase::InitializeFromParams,);

    /** 应用修改参数 */
    virtual void ApplyModification(const FSyInstancedStruct& ModificationParams) PURE_VIRTUAL(USyStateMetadataBase::ApplyModification,);

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
 * USyTagStateMetadata - 标签状态元数据
 * 
 * 用于存储GameplayTag类型的状态值
 */
UCLASS(Blueprintable)
class SYSTATECORE_API USyTagStateMetadata : public USyStateMetadataBase
{
    GENERATED_BODY()

public:
    /** 构造函数 */
    USyTagStateMetadata();

    /** 从参数初始化元数据 */
    virtual void InitializeFromParams(const FSyInstancedStruct& InitParams) override;

    /** 应用修改参数 */
    virtual void ApplyModification(const FSyInstancedStruct& ModificationParams) override;

    /** 获取标签值 */
    UFUNCTION(BlueprintPure, Category = "SyStateCore|TagStateMetadata")
    FGameplayTag GetValue() const { return Value; }

    /** 设置标签值 */
    UFUNCTION(BlueprintCallable, Category = "SyStateCore|TagStateMetadata")
    void SetValue(const FGameplayTag& InValue) { Value = InValue; }

protected:
    /** 标签值 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyStateCore|TagStateMetadata")
    FGameplayTag Value;
};
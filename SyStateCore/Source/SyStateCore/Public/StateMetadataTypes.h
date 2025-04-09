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
 * 每个子类必须实现 GetValueDataType 以标识其管理的具体数据类型
 */
UCLASS(Abstract, Blueprintable)
class SYSTATECORE_API USyStateMetadataBase : public UO_TagMetadata
{
    GENERATED_BODY()

public:
    /** 构造函数 */
    USyStateMetadataBase();

    /** 返回此元数据管理的具体数据类型 (USTRUCT) */
    virtual UScriptStruct* GetValueDataType() const PURE_VIRTUAL(USyStateMetadataBase::GetValueDataType, return nullptr;);

    /** 从参数初始化元数据 */
    virtual void InitializeFromParams(const FSyInstancedStruct& InitParams);

    /** 应用修改参数 */
    virtual void ApplyModification(const FSyInstancedStruct& ModificationParams);

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

    /** 子类实现：更新值的具体逻辑 */
    virtual void UpdateValue(const FSyInstancedStruct& Params) PURE_VIRTUAL(USyStateMetadataBase::UpdateValue,);

    /** 辅助方法：验证参数类型并调用子类实现 */
    bool ValidateAndProcessParams(const FSyInstancedStruct& Params, const TCHAR* OperationName);
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

    /** 返回此元数据管理的具体数据类型 (FGameplayTag) */
    virtual UScriptStruct* GetValueDataType() const override;

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

    /** 更新值的具体逻辑 */
    virtual void UpdateValue(const FSyInstancedStruct& Params) override;
};
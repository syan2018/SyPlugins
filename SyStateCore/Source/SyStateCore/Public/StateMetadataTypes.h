// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "O_TagMetadata.h"
#include "StructUtils/InstancedStruct.h"
#include "StateMetadataTypes.generated.h"

/**
 * USyStateMetadataBase - 状态元数据基类
 * 
 * 所有状态元数据的基类，继承自UO_TagMetadata
 * 提供状态标签和参数处理的基本功能
 * 每个子类必须实现 GetValueDataType 以标识其管理的具体数据类型
 * 通常每个USyStateMetadata与一个USTRUCT关联，用于存储具体数据
 */
UCLASS(Abstract, Blueprintable)
class SYSTATECORE_API USyStateMetadataBase : public UO_TagMetadata
{
    GENERATED_BODY()

public:
    /** 构造函数 */
    USyStateMetadataBase();

    /** 返回此元数据管理的具体数据类型 (USTRUCT)
     *  用于编辑器下自适应 FInstancedStruct
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SyStateCore|StateMetadata")
    UScriptStruct* GetValueDataType() const;
    virtual UScriptStruct* GetValueDataType_Implementation() const { return nullptr; }

    /** 从参数初始化元数据 */
    UFUNCTION(BlueprintCallable, Category = "SyStateCore|StateMetadata")
    virtual void InitializeFromParams(const FInstancedStruct& InitParams);

    /** 应用修改参数 */
    UFUNCTION(BlueprintCallable, Category = "SyStateCore|StateMetadata")
    virtual void ApplyModification(const FInstancedStruct& ModificationParams);

    /** 获取状态标签 */
    UFUNCTION(BlueprintPure, Category = "SyStateCore|StateMetadata")
    FGameplayTag GetStateTag() const { return StateTag; }

    /** 设置状态标签 */
    UFUNCTION(BlueprintCallable, Category = "SyStateCore|StateMetadata")
    void SetStateTag(const FGameplayTag& InStateTag) { StateTag = InStateTag; }

    /** 获取存储的值（返回 FInstancedStruct） */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SyStateCore|StateMetadata")
    FInstancedStruct GetValueStruct() const;
    virtual FInstancedStruct GetValueStruct_Implementation() const { return FInstancedStruct(); }

    /** 设置存储的值（使用 FInstancedStruct） */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SyStateCore|StateMetadata")
    void SetValueStruct(const FInstancedStruct& InValue);
    virtual void SetValueStruct_Implementation(const FInstancedStruct& InValue);
    

    /** 尝试获取特定类型的值（C++ 使用） */
    template<typename T>
    bool TryGetValue(T& OutValue) const
    {
        FInstancedStruct ValueStruct = GetValueStruct();
        if (ValueStruct.IsValid() && ValueStruct.GetScriptStruct() == T::StaticStruct())
        {
            if (const T* ValuePtr = ValueStruct.GetPtr<T>())
            {
                OutValue = *ValuePtr;
                return true;
            }
        }
        return false;
    }

    /** 获取特定类型的值（C++ 使用） */
    template<typename T>
    T GetValue() const
    {
        T Result;
        TryGetValue(Result);
        return Result;
    }

    /** 设置特定类型的值（C++ 使用） */
    template<typename T>
    void SetValue(const T& InValue)
    {
        FInstancedStruct ValueStruct;
        ValueStruct.InitializeAs<T>(InValue);
        SetValueStruct(ValueStruct);
    }

protected:
    /** 状态标签 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyStateCore|StateMetadata")
    FGameplayTag StateTag;

    /** 辅助方法：验证参数类型并调用子类实现 */
    bool ValidateAndProcessParams(const FInstancedStruct& Params, const TCHAR* OperationName);
};


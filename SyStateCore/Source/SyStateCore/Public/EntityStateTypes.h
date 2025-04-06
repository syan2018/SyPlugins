// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SyCore/Public/Foundation/Utilities/SyInstancedStruct.h"
#include "O_TagMetadata.h"
#include "EntityStateTypes.generated.h"

/**
 * FSyStateMetadataArray - 状态元数据数组
 * 
 * 用于存储特定状态标签的多个元数据对象
 */
USTRUCT(BlueprintType)
struct SYSTATECORE_API FSyStateMetadataArray
{
    GENERATED_BODY()

    /** 默认构造函数 */
    FSyStateMetadataArray() = default;

    /** 从元数据对象数组构造 */
    FSyStateMetadataArray(const TArray<TObjectPtr<UO_TagMetadata>>& InMetadataArray)
        : MetadataArray(InMetadataArray) {}

    /** 元数据对象数组 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SyStateCore|StateMetadata")
    TArray<TObjectPtr<UO_TagMetadata>> MetadataArray;

    /** 添加元数据对象 */
    void AddMetadata(UO_TagMetadata* Metadata)
    {
        if (Metadata)
        {
            MetadataArray.Add(Metadata);
        }
    }

    /** 添加多个元数据对象 */
    void AddMetadataArray(const TArray<TObjectPtr<UO_TagMetadata>>& InMetadataArray)
    {
        MetadataArray.Append(InMetadataArray);
    }

    /** 清除所有元数据对象 */
    void ClearMetadata()
    {
        MetadataArray.Empty();
    }
};

/**
 * FSyStateParams - 状态参数
 * 
 * 用于存储对特定状态标签的多个参数
 */
USTRUCT(BlueprintType)
struct SYSTATECORE_API FSyStateParams
{
    GENERATED_BODY()

    /** 默认构造函数 */
    FSyStateParams() = default;

    /** 从参数数组构造 */
    FSyStateParams(const TArray<FSyInstancedStruct>& InParams)
        : Params(InParams) {}

    /** 参数数组 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyStateCore|StateParams")
    TArray<FSyInstancedStruct> Params;

    /** 添加参数 */
    void AddParam(const FSyInstancedStruct& Param)
    {
        Params.Add(Param);
    }

    /** 添加多个参数 */
    void AddParams(const TArray<FSyInstancedStruct>& InParams)
    {
        Params.Append(InParams);
    }

    /** 清除所有参数 */
    void ClearParams()
    {
        Params.Empty();
    }
};

/**
 * FSyEntityState - 实体状态
 * 
 * 存储实体的完整状态数据，使用GameplayTag作为键，TagMetadata对象作为值
 */
USTRUCT(BlueprintType)
struct SYSTATECORE_API FSyEntityState
{
    GENERATED_BODY()

    /** 默认构造函数 */
    FSyEntityState() = default;

    /** 状态数据映射：状态标签 -> TagMetadata对象数组 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SyStateCore|EntityState", Transient)
    TMap<FGameplayTag, FSyStateMetadataArray> StateData;

    /** 查找指定标签的第一个指定类型的元数据对象 */
    template<typename T>
    T* FindFirstStateMetadata(const FGameplayTag& StateTag) const
    {
        if (const FSyStateMetadataArray* MetadataArray = StateData.Find(StateTag))
        {
            for (const TObjectPtr<UO_TagMetadata>& Metadata : MetadataArray->MetadataArray)
            {
                if (T* TypedMetadata = Cast<T>(Metadata))
                {
                    return TypedMetadata;
                }
            }
        }
        return nullptr;
    }

    /** 查找指定标签的所有指定类型的元数据对象 */
    template<typename T>
    TArray<T*> GetAllStateMetadata(const FGameplayTag& StateTag) const
    {
        TArray<T*> Result;
        if (const FSyStateMetadataArray* MetadataArray = StateData.Find(StateTag))
        {
            for (const TObjectPtr<UO_TagMetadata>& Metadata : MetadataArray->MetadataArray)
            {
                if (T* TypedMetadata = Cast<T>(Metadata))
                {
                    Result.Add(TypedMetadata);
                }
            }
        }
        return Result;
    }

    /** 添加状态元数据对象 */
    void AddStateMetadata(const FGameplayTag& StateTag, UO_TagMetadata* Metadata)
    {
        if (Metadata)
        {
            FSyStateMetadataArray& MetadataArray = StateData.FindOrAdd(StateTag);
            MetadataArray.AddMetadata(Metadata);
        }
    }

    /** 移除状态元数据对象 */
    bool RemoveStateMetadata(const FGameplayTag& StateTag, UO_TagMetadata* Metadata)
    {
        if (FSyStateMetadataArray* MetadataArray = StateData.Find(StateTag))
        {
            return MetadataArray->MetadataArray.Remove(Metadata) > 0;
        }
        return false;
    }

    /** 清除指定标签的所有状态元数据对象 */
    void ClearStateMetadata(const FGameplayTag& StateTag)
    {
        StateData.Remove(StateTag);
    }

    /** 清除所有状态元数据对象 */
    void ClearAllStateMetadata()
    {
        StateData.Empty();
    }

    /** 批量应用初始化数据 */
    void ApplyInitData(const FSyEntityInitData& InitData);

    /** 批量应用状态修改 */
    void ApplyStateModifications(const TMap<FGameplayTag, FSyStateParams>& StateModifications);
};

/**
 * FSyEntityInitData - 实体初始化数据
 * 
 * 定义实体初始化时的基础状态配置
 */
USTRUCT(BlueprintType)
struct SYSTATECORE_API FSyEntityInitData
{
    GENERATED_BODY()

    /** 默认构造函数 */
    FSyEntityInitData() = default;

    /** 初始状态映射：状态标签 -> 初始化参数数组 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyStateCore|EntityInitData")
    TMap<FGameplayTag, FSyStateParams> InitialState;

    /** 添加初始状态参数 */
    void AddInitialStateParam(const FGameplayTag& StateTag, const FSyInstancedStruct& InitParam)
    {
        FSyStateParams& StateParams = InitialState.FindOrAdd(StateTag);
        StateParams.AddParam(InitParam);
    }

    /** 添加多个初始状态参数 */
    void AddInitialStateParams(const FGameplayTag& StateTag, const TArray<FSyInstancedStruct>& InitParams)
    {
        FSyStateParams& StateParams = InitialState.FindOrAdd(StateTag);
        StateParams.AddParams(InitParams);
    }

    /** 清除指定标签的所有初始状态参数 */
    void ClearInitialStateParams(const FGameplayTag& StateTag)
    {
        InitialState.Remove(StateTag);
    }

    /** 清除所有初始状态参数 */
    void ClearAllInitialStateParams()
    {
        InitialState.Empty();
    }
}; 
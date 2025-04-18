// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "O_TagMetadata.h" // Needed for UO_TagMetadata
#include "StateParameterTypes.h" // Needed for FSyStateParams and FSyStateParameterSet
#include "StateContainerTypes.generated.h"

// Forward Declarations
struct FSyStateParameterSet; // Forward declare the renamed struct

/**
 * FSyStateMetadatas - 状态元数据数组
 * 
 * 用于存储特定状态标签的多个元数据对象
 * 处理运行时元数据对象 UObject
 */
USTRUCT(BlueprintType)
struct SYSTATECORE_API FSyStateMetadatas
{
	GENERATED_BODY()

	/** 默认构造函数 */
	FSyStateMetadatas() = default;

	/** 从元数据对象数组构造 */
	FSyStateMetadatas(const TArray<TObjectPtr<UO_TagMetadata>>& InMetadataArray)
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
 * FSyStateCategories - 状态集合
 * 
 * 存储完整状态数据，使用GameplayTag作为键，TagMetadata对象作为值 
 * 后续常用作储存实体状态，以及实现 StateManager 中相关容器
 */
USTRUCT(BlueprintType)
struct SYSTATECORE_API FSyStateCategories
{
	GENERATED_BODY()

	/** 默认构造函数 */
	FSyStateCategories() = default;

	/** 状态数据映射：状态标签 -> TagMetadata对象数组 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SyStateCore|EntityState", Transient)
	TMap<FGameplayTag, FSyStateMetadatas> StateData;

	/** 查找指定标签的第一个指定类型的元数据对象 */
	template<typename T>
	T* FindFirstStateMetadata(const FGameplayTag& StateTag) const
	{
		if (const FSyStateMetadatas* MetadataArrayPtr = StateData.Find(StateTag))
		{
			for (const TObjectPtr<UO_TagMetadata>& Metadata : MetadataArrayPtr->MetadataArray)
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
		 if (const FSyStateMetadatas* MetadataArrayPtr = StateData.Find(StateTag))
        {
            for (const TObjectPtr<UO_TagMetadata>& Metadata : MetadataArrayPtr->MetadataArray)
            {
                if (T* TypedMetadata = Cast<T>(Metadata))
                {
                    Result.Add(TypedMetadata);
                }
            }
        }
		return Result;
	}

	/** 添加状态元数据对象 (内部使用，可能被 AddOrUpdateMetadataParam 取代) */
	void AddStateMetadata(const FGameplayTag& StateTag, UO_TagMetadata* Metadata)
	{
		if (Metadata)
		{
			FSyStateMetadatas& MetadataArray = StateData.FindOrAdd(StateTag);
			MetadataArray.AddMetadata(Metadata);
		}
	}

	/** 移除状态元数据对象 */
	bool RemoveStateMetadata(const FGameplayTag& StateTag, UO_TagMetadata* Metadata)
	{
		if (FSyStateMetadatas* MetadataArrayPtr = StateData.Find(StateTag))
		{
			return MetadataArrayPtr->MetadataArray.Remove(Metadata) > 0;
		}
		return false;
	}

	/** 清除指定标签的所有状态元数据对象 */
	void ClearStateMetadata(const FGameplayTag& StateTag)
	{
		StateData.Remove(StateTag);
	}

	/** 清除所有状态数据 */
	void Empty()
	{
		StateData.Empty();
	}

	/** 批量应用初始化数据 */
    void ApplyInitData(const FSyStateParameterSet& InitData);

    /** 
     * @brief 从参数Map智能更新状态，尝试复用现有元数据对象。
     * @param AggregatedParamsMap 包含聚合后状态参数的Map。
     */
    void UpdateFromParameterMap(const TMap<FGameplayTag, TArray<FInstancedStruct>>& AggregatedParamsMap);

    /** 获取内部数据Map的常量引用 (供 GetEffectiveStateCategories 使用) */
	const TMap<FGameplayTag, FSyStateMetadatas>& GetStateDataMap() const { return StateData; }

	/** 添加或更新指定Tag下的元数据 */
	void AddOrUpdateMetadataParam(const FGameplayTag& StateTag, const TArray<FInstancedStruct>& NewParams);

	/**
	 * @brief 将另一个状态集合合并到当前集合中。
	 *        来自 'Other' 的状态标签会覆盖当前集合中已存在的同名标签。
	 *        仅存在于 'Other' 中的标签会被添加进来。
	 * @param Other 要合并进来的状态集合 (其状态具有更高优先级)。
	 */
	void MergeWith(const FSyStateCategories& Other);
}; 
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "O_TagMetadata.h" // Needed for UO_TagMetadata
#include "Core/StateParameterTypes.h" // Needed for FSyStateParams and FSyStateParameterSet
#include "StateContainerTypes.generated.h"

// Forward Declarations
struct FSyStateParameterSet; // Forward declare the renamed struct

/**
 * ESyStateLayer - 状态层级枚举
 * 
 * 定义状态的优先级层级，高层级的状态会覆盖低层级的状态
 * 用于实现复杂的状态管理场景（如Buff系统）
 */
UENUM(BlueprintType)
enum class ESyStateLayer : uint8
{
	/** 默认层：初始默认值，优先级最低 */
	Default = 0 UMETA(DisplayName = "Default"),
	
	/** 持久层：从 StateManager 同步的全局状态 */
	Persistent = 1 UMETA(DisplayName = "Persistent"),
	
	/** 临时层：临时修改（如Buff/Debuff），会被保存到存档 */
	Temporary = 2 UMETA(DisplayName = "Temporary"),
	
	/** 覆盖层：强制覆盖，优先级最高（如GM命令） */
	Override = 3 UMETA(DisplayName = "Override"),
	
	MAX UMETA(Hidden)
};

/**
 * FSyStateMetadatas - 状态元数据数组
 * 
 * 用于存储特定状态标签的多个元数据对象
 * 处理运行时元数据对象 UObject
 */
USTRUCT(BlueprintType)
struct SYSTATESYSTEM_API FSyStateMetadatas
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
struct SYSTATESYSTEM_API FSyStateCategories
{
	GENERATED_BODY()

	/** 默认构造函数 */
	FSyStateCategories() = default;

	/** 状态数据映射：状态标签 -> TagMetadata对象数组 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SyStateCore|EntityState")
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
	
	/**
	 * @brief 自定义序列化函数，确保 UObject 元数据正确序列化
	 */
	bool Serialize(FArchive& Ar);
	
	/**
	 * @brief 序列化后处理，用于重建对象引用
	 */
	void PostSerialize(const FArchive& Ar);
};

// 添加序列化支持
template<>
struct TStructOpsTypeTraits<FSyStateCategories> : public TStructOpsTypeTraitsBase2<FSyStateCategories>
{
	enum
	{
		WithSerializer = true,
		WithPostSerialize = true,
	};
};

/**
 * FSyLayeredStateContainer - 分层状态容器
 * 
 * 支持多层级的状态管理，按优先级合并状态
 * 用于 USyStateComponent 实现复杂的状态覆盖逻辑
 */
USTRUCT(BlueprintType)
struct SYSTATESYSTEM_API FSyLayeredStateContainer
{
	GENERATED_BODY()

	/** 默认构造函数 */
	FSyLayeredStateContainer() = default;

	/**
	 * @brief 获取指定层级的状态容器
	 * @param Layer 状态层级
	 * @return 该层级的状态容器引用
	 */
	FSyStateCategories& GetLayer(ESyStateLayer Layer);

	/**
	 * @brief 获取指定层级的状态容器（只读）
	 * @param Layer 状态层级
	 * @return 该层级的状态容器常量引用
	 */
	const FSyStateCategories& GetLayer(ESyStateLayer Layer) const;

	/**
	 * @brief 设置指定层级的完整状态
	 * @param Layer 状态层级
	 * @param NewState 新的状态数据
	 */
	void SetLayer(ESyStateLayer Layer, const FSyStateCategories& NewState);

	/**
	 * @brief 清除指定层级的所有状态
	 * @param Layer 状态层级
	 */
	void ClearLayer(ESyStateLayer Layer);

	/**
	 * @brief 获取合并后的有效状态（按优先级从低到高合并）
	 * @return 所有层级合并后的最终状态
	 */
	FSyStateCategories GetEffectiveState() const;

	/**
	 * @brief 检查指定层级是否有数据
	 * @param Layer 状态层级
	 * @return 如果该层级有状态数据返回 true
	 */
	bool HasDataInLayer(ESyStateLayer Layer) const;

	/**
	 * @brief 清除所有层级的状态
	 */
	void ClearAllLayers();

	/**
	 * @brief 在指定层级应用参数集（合并模式）
	 * @param Layer 目标层级
	 * @param ParamSet 要应用的参数集
	 */
	void ApplyParameterSetToLayer(ESyStateLayer Layer, const FSyStateParameterSet& ParamSet);

	/**
	 * @brief 从特定层级查找状态元数据
	 * @param Layer 状态层级
	 * @param StateTag 状态标签
	 * @return 找到的元数据对象，找不到返回 nullptr
	 */
	template<typename T>
	T* FindStateMetadataInLayer(ESyStateLayer Layer, const FGameplayTag& StateTag) const;

private:
	/** 各层级的状态容器 */
	UPROPERTY(VisibleAnywhere, Category = "SyStateCore|LayeredState")
	TMap<ESyStateLayer, FSyStateCategories> StateLayers;

	/** 缓存的有效状态（用于性能优化） */
	mutable FSyStateCategories CachedEffectiveState;

	/** 缓存版本号（用于失效检测） */
	mutable int32 CacheVersion = 0;

	/** 当前版本号（每次修改递增） */
	int32 CurrentVersion = 0;

	/** 使缓存失效 */
	void InvalidateCache();
};

// 模板实现
template<typename T>
T* FSyLayeredStateContainer::FindStateMetadataInLayer(ESyStateLayer Layer, const FGameplayTag& StateTag) const
{
	if (const FSyStateCategories* LayerState = StateLayers.Find(Layer))
	{
		return LayerState->FindFirstStateMetadata<T>(StateTag);
	}
	return nullptr;
} 
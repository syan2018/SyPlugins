// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SyCore/Public/Foundation/Utilities/SyInstancedStruct.h" // Needed for FSyInstancedStruct
#include "StateParameterTypes.generated.h"



/**
 * FSyStateParams - 状态参数
 * 
 * 用于存储对特定状态标签的多个参数
 * 现在包含 Tag 本身，以便在数组中使用
 */
USTRUCT(BlueprintType)
struct SYSTATECORE_API FSyStateParams
{
	
	GENERATED_BODY()

	/** 关联的状态标签 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyStateCore|StateParams")
	FGameplayTag Tag;
	
	/** 参数数组 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyStateCore|StateParams", meta = (BaseStruct = "/Script/SyCore.SyBaseInstancedStruct", ExcludeBaseStruct)) // ShowOnlyInnerProperties might be useful later
	TArray<FInstancedStruct> Params;

	/**
	 *	TODO: 重构！！！被 UE 气晕, 需要找到手段强制序列化(UPROPERTY不够)，不然重载稳定丢失 
	 *	除 PostSerialize 目前没地方正常拿 Tag 更新回调，除非变 UObject 然后 PostEditChangeProperty 
	 *	且要求加 Anywhere 系 Flag，不然在关卡编辑器中对象 PostSerialize 时有一次调用会变成 None 
	 */
	UPROPERTY(VisibleAnywhere, meta = (EditCondition = "false", EditConditionHides))
	FGameplayTag LastTag = FGameplayTag::EmptyTag;

	/** 默认构造函数 */
	FSyStateParams() = default;
	
	/** 从标签和参数数组构造 */
	FSyStateParams(const FGameplayTag& InTag, const TArray<FSyInstancedStruct>& InParams = TArray<FSyInstancedStruct>())
		: Tag(InTag), Params(InParams) {}

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

	/**
	 * PostSerialize - 序列化后处理
	 * 
	 * 在序列化完成后调用，用于确保 Tag 和参数的一致性
	 * 当 Tag 发生变化时，会自动更新参数数组
	 * 
	 * @param Ar 序列化存档
	 */
	void PostSerialize(const FArchive& Ar);
	
};

// 为 FSyStateParams 添加 TStructOpsTypeTraits 支持
template<>
struct TStructOpsTypeTraits<FSyStateParams> : public TStructOpsTypeTraitsBase2<FSyStateParams>
{
	enum
	{
		WithPostSerialize = true,
	};
};

/**
 * FSyStateParameterSet - 状态参数集
 * 
 * 定义一组与状态相关的参数，现在使用 TArray 存储。
 * 提供一个接口以 TMap 形式获取数据，用于兼容旧代码。
 * TODO: 实现 FSyStateParams 的编辑器拓展，选择 Tag 自动初始化 Params 类型，并处理重复 Tag。
 */
USTRUCT(BlueprintType)
struct SYSTATECORE_API FSyStateParameterSet
{
	GENERATED_BODY()

	/** 状态参数数组 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyStateCore|StateParameterSet", meta = (TitleProperty="Tag")) // Use Tag as title in array view
	TArray<FSyStateParams> Parameters;

	/** 默认构造函数 */
	FSyStateParameterSet() = default;

	/** 
     * 从 TMap<Tag, ParamArray> 构造 FSyStateParameterSet。
     * @param InMap 输入的 TMap，Key 是 Tag，Value 是 FInstancedStruct 参数数组。
     */
	explicit FSyStateParameterSet(const TMap<FGameplayTag, TArray<FSyInstancedStruct>>& InMap)
	{
		Parameters.Empty(InMap.Num()); // Clear and reserve
		for (const auto& Pair : InMap)
		{
			if (Pair.Key.IsValid())
			{
				Parameters.Emplace(Pair.Key, Pair.Value); // Use Emplace for efficiency
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("FSyStateParameterSet TMap constructor: Skipping invalid tag found as key in the input map."));
			}
		}
	}
    
    /** 
     * 从 TMap<Tag, ParamArray> 赋值。
     * @param InMap 输入的 TMap。
     * @return 对自身的引用。
     */
    FSyStateParameterSet& operator=(const TMap<FGameplayTag, TArray<FSyInstancedStruct>>& InMap)
    {
        Parameters.Empty(InMap.Num()); // Clear and reserve
        for (const auto& Pair : InMap)
        {
            if (Pair.Key.IsValid())
            {
                Parameters.Emplace(Pair.Key, Pair.Value);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("FSyStateParameterSet TMap assignment: Skipping invalid tag found as key in the input map."));
            }
        }
        return *this;
    }

	/** 清除所有状态参数 */
	void ClearAllStateParams()
	{
		Parameters.Empty();
	}

	/** 
	 * 以 TMap 的形式获取参数集，用于兼容旧代码。
	 * 注意：如果数组中存在重复的 Tag，此函数只会保留最后一个遇到的 Tag 对应的参数。
	 * @param bLogOnDuplicate 如果为 true，并且检测到重复 Tag，则会输出警告日志。
	 * @return 一个包含状态参数的 TMap。
	 */
	TMap<FGameplayTag, TArray<FInstancedStruct>> GetParametersAsMap(bool bLogOnDuplicate = true) const
	{
		TMap<FGameplayTag, TArray<FInstancedStruct>> ResultMap;
		for (const FSyStateParams& StateParams : Parameters)
		{
			if (ResultMap.Contains(StateParams.Tag))
			{
				if (bLogOnDuplicate)
				{
					UE_LOG(LogTemp, Warning, TEXT("FSyStateParameterSet::GetParametersAsMap: Duplicate tag '%s' found in the array. Overwriting previous entry in the returned map."), *StateParams.Tag.ToString());
				}
			}
			// Add or overwrite the entry
			ResultMap.Add(StateParams.Tag, StateParams.Params);
		}
		return ResultMap;
	}

	/** 
	 * 根据 Tag 查找第一个匹配的 FSyStateParams (运行时辅助函数)
	 * @param TagToFind 要查找的 Tag
	 * @return 指向找到的 FSyStateParams 的指针，如果未找到则为 nullptr。
	 */
	FSyStateParams* FindStateParams(const FGameplayTag& TagToFind)
	{
		return Parameters.FindByPredicate([&TagToFind](const FSyStateParams& Item){ return Item.Tag == TagToFind; });
	}

	/** 
	 * 根据 Tag 查找第一个匹配的 FSyStateParams (运行时辅助函数, const 版本)
	 * @param TagToFind 要查找的 Tag
	 * @return 指向找到的 FSyStateParams 的 const 指针，如果未找到则为 nullptr。
	 */
	const FSyStateParams* FindStateParams(const FGameplayTag& TagToFind) const
	{
		return Parameters.FindByPredicate([&TagToFind](const FSyStateParams& Item){ return Item.Tag == TagToFind; });
	}

	// --- Compatibility Interfaces ---

	/** 
     * 添加或更新指定 Tag 的单个状态参数。
     * 如果 Tag 已存在，参数将添加到现有条目的 Params 数组中。
     * 如果 Tag 不存在，将创建一个新的 FSyStateParams 条目。
     * @param StateTag 要关联的 Tag。
     * @param Param 要添加的参数。
     */
	void AddStateParam(const FGameplayTag& StateTag, const FSyInstancedStruct& Param)
	{
		if (!StateTag.IsValid()) return;
		FSyStateParams* ExistingParams = FindStateParams(StateTag);
		if (ExistingParams)
		{
			ExistingParams->AddParam(Param);
		}
		else
		{
			Parameters.Emplace(StateTag, TArray<FSyInstancedStruct>{Param});
		}
	}

	/** 
     * 添加或更新指定 Tag 的多个状态参数。
     * 如果 Tag 已存在，参数将附加到现有条目的 Params 数组中。
     * 如果 Tag 不存在，将创建一个新的 FSyStateParams 条目。
     * @param StateTag 要关联的 Tag。
     * @param InParams 要添加的参数数组。
     */
	void AddStateParams(const FGameplayTag& StateTag, const TArray<FSyInstancedStruct>& InParams)
	{
		if (!StateTag.IsValid() || InParams.Num() == 0) return;
		FSyStateParams* ExistingParams = FindStateParams(StateTag);
		if (ExistingParams)
		{
			ExistingParams->AddParams(InParams);
		}
		else
		{
			Parameters.Emplace(StateTag, InParams);
		}
	}

	/** 
     * 移除指定 Tag 的所有状态参数条目。
     * @param StateTag 要移除参数的 Tag。
     * @return 移除了多少个条目 (对于 TArray，可能>1，但通常我们期望编辑器防止重复)
     */
	int32 RemoveStateParams(const FGameplayTag& StateTag)
	{
		if (!StateTag.IsValid()) return 0;
		return Parameters.RemoveAll([&StateTag](const FSyStateParams& Item){ return Item.Tag == StateTag; });
	}

}; 
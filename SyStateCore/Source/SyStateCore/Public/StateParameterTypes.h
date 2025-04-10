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
 * FSyStateParameterSet - 状态参数集
 * 
 * 定义一组与状态相关的参数，通常用于初始化或修改状态。
 * TODO: 实现编辑器拓展，选择 Tag 自动初始化 StateParams 类型 
 */
USTRUCT(BlueprintType)
struct SYSTATECORE_API FSyStateParameterSet
{
	GENERATED_BODY()

	/** 默认构造函数 */
	FSyStateParameterSet() = default;

	/** 状态参数映射：状态标签 -> 参数数组 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyStateCore|StateParameterSet")
	TMap<FGameplayTag, FSyStateParams> Parameters; // Renamed from InitialState

	/** 获取状态参数映射的常量引用 */
	const TMap<FGameplayTag, FSyStateParams>& GetStateParamsMap() const
	{
		return Parameters;
	}

	/** 添加状态参数 */
	void AddStateParam(const FGameplayTag& StateTag, const FSyInstancedStruct& Param)
	{
		FSyStateParams& StateParams = Parameters.FindOrAdd(StateTag); // Use Parameters map
		StateParams.AddParam(Param);
	}

	/** 添加多个状态参数 */
	void AddStateParams(const FGameplayTag& StateTag, const TArray<FSyInstancedStruct>& InParams)
	{
		FSyStateParams& StateParams = Parameters.FindOrAdd(StateTag); // Use Parameters map
		StateParams.AddParams(InParams);
	}

	/** 清除指定标签的所有状态参数 */
	void ClearStateParams(const FGameplayTag& StateTag)
	{
		Parameters.Remove(StateTag); // Use Parameters map
	}

	/** 清除所有状态参数 */
	void ClearAllStateParams()
	{
		Parameters.Empty(); // Use Parameters map
	}
}; 
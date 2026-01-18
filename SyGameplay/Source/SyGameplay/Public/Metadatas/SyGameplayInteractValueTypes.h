#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h" // For FGameplayTag
#include "StateTreeReference.h"   // For FStateTreeReference
#include "StructUtils/InstancedStruct.h" // For FInstancedStruct
#include "Foundation/SyInstancedStruct.h" // For FSyBaseInstancedStruct
#include "Core/Metadatas/ListMetadataValueTypes.h" // For FSyListParameterBase (needs SyStateSystem dependency)
#include "UObject/SoftObjectPtr.h" // For TSoftObjectPtr
#include "FlowAsset.h"
#include "SyGameplayInteractValueTypes.generated.h"

// Forward declaration for Flow Asset (avoids direct include dependency in header if possible)
// class UFlowAsset; 

/**
 * @brief 交互信息的基础结构体。
 * 所有具体的交互信息结构体都应从此继承。
 */
USTRUCT(BlueprintType)
struct SYGAMEPLAY_API FSyInteractInfoBase : public FSyBaseInstancedStruct
{
	GENERATED_BODY()

	/** 标识此交互信息的具体类型，便于区分和处理 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Info")
	FGameplayTag InteractTypeTag;
	
};

/**
 * @brief 基础交互信息，包含显示名称和描述。
 */
USTRUCT(BlueprintType)
struct SYGAMEPLAY_API FSyBasicInteractInfo : public FSyInteractInfoBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Info")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Info", meta=(MultiLine="true"))
	FText Description;

	FSyBasicInteractInfo()
	{
		InteractTypeTag = FGameplayTag::RequestGameplayTag(FName("Interaction.Basic")); // Example Tag
	}
};

/**
 * @brief Flow 交互信息，用于触发 Flow Graph。
 */
USTRUCT(BlueprintType)
struct SYGAMEPLAY_API FSyFlowInteractInfo : public FSyInteractInfoBase
{
	GENERATED_BODY()

	/** 要触发的 Flow 资源 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Info", meta = (AllowedClasses = "/Script/Flow.FlowAsset"))
	TSoftObjectPtr<UFlowAsset> FlowAsset; // Requires Flow module dependency

	/** 传递给 Flow 的输入数据 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Info", meta=(BaseStruct="/Script/SyStateCore.SyBaseInstancedStruct", ExcludeBaseStruct))
	FInstancedStruct InputPayload; 

	FSyFlowInteractInfo()
	{
		InteractTypeTag = FGameplayTag::RequestGameplayTag(FName("Interaction.Flow")); // Example Tag
	}
};

/**
 * @brief StateTree 交互信息，用于与 StateTree 交互。
 */
USTRUCT(BlueprintType)
struct SYGAMEPLAY_API FSyStateTreeInteractInfo : public FSyInteractInfoBase
{
	GENERATED_BODY()

	/** 关联的 StateTree 引用 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Info")
	FStateTreeReference StateTreeRef; // Requires StateTreeModule dependency

	/** 用于触发 StateTree 的特定 Tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction Info")
	FGameplayTag TriggerTag;

	FSyStateTreeInteractInfo()
	{
		InteractTypeTag = FGameplayTag::RequestGameplayTag(FName("Interaction.StateTree")); // Example Tag
	}
};

/**
 * @brief 存储交互信息列表的值类型结构体。
 * 继承自 FSyListParameterBase 以支持列表聚合逻辑。
 * 直接包含带有类型过滤元数据的交互信息数组，供编辑器使用。
 */
USTRUCT(BlueprintType) 
struct SYGAMEPLAY_API FSyInteractionListValue : public FSyListParameterBase // 继承基类
{
    GENERATED_BODY()

    /** 存储列表元素的数组，限制只能添加 FSyInteractInfoBase 的子类 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="List Value", meta=(DisplayName="Interaction Items", BaseStruct="/Script/SyGameplay.FSyInteractInfoBase", ExcludeBaseStruct, ShowTreeView))
    TArray<FInstancedStruct> InteractionItems; // 具体的列表实现，带有编辑器元数据

    // --- 重写基类虚函数，使其操作 InteractionItems --- 

    virtual TArray<FInstancedStruct>& GetListItemsInternal() override
    {
        return InteractionItems; // 返回自己的列表
    }

    virtual const TArray<FInstancedStruct>& GetListItemsInternal() const override
    {
        return InteractionItems; // 返回自己的列表 (const)
    }

    virtual void AggregateItemsInternal(const TArray<FInstancedStruct>& SourceItems) override
    {
        // 实现追加逻辑
        // 可以添加额外的检查，确保只追加有效的 FSyInteractInfoBase 实例
        for(const FInstancedStruct& Item : SourceItems)
        {
            if(Item.IsValid() && Item.GetScriptStruct()->IsChildOf(FSyInteractInfoBase::StaticStruct()))
            {
                InteractionItems.Add(Item);
            }
            else
            {
                 UE_LOG(LogTemp, Warning, TEXT("FSyInteractionListValue::AggregateItemsInternal: Skipped adding an invalid or non-FSyInteractInfoBase struct during aggregation."));
            }
        }
        // 或者直接追加，如果信任源数据
        // InteractionItems.Append(SourceItems);
    }

	// 移除旧的辅助函数，因为现在依赖虚函数
	/*
	bool AddInteraction(const FInstancedStruct& InteractInfoInstance)
	{
		if (InteractInfoInstance.IsValid() && InteractInfoInstance.GetScriptStruct()->IsChildOf(FSyInteractInfoBase::StaticStruct()))
		{
			InteractionItems.Add(InteractInfoInstance);
			return true;
		}
		// 可以添加日志记录无效的添加尝试
		return false;
	}

	void ClearInteractions()
	{
		InteractionItems.Empty();
	}
	*/

	// 可以在这里添加其他特定于交互列表的 UFUNCTION 或辅助函数（如果需要的话）
};

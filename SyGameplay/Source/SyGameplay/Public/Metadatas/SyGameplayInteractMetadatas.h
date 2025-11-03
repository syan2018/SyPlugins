#pragma once

#include "CoreMinimal.h"
#include "Core/StateMetadataTypes.h"
#include "SyGameplayInteractValueTypes.h" // The value type FSyInteractionListValue
#include "SyGameplayInteractMetadatas.generated.h"

/**
 * @brief 状态元数据，用于存储和管理一个交互信息列表 (FSyInteractionListValue)。
 *
 * 这个元数据允许一个状态（例如，一个实体上的状态）持有一系列交互信息，
 * 每条信息可以是不同的类型（基础、Flow、StateTree 等），由 FSyInteractInfoBase 及其子类定义。
 */
UCLASS(Blueprintable, Category = "SyGameplay|Metadata")
class SYGAMEPLAY_API USyInteractionListMetadata : public USyStateMetadataBase
{
	GENERATED_BODY()

public:
	/** 返回此元数据管理的具体数据类型 (FSyInteractionListValue) */
	virtual UScriptStruct* GetValueDataType_Implementation() const override;

	/** 获取存储的值 (FSyInteractionListValue) */
	virtual FInstancedStruct GetValueStruct_Implementation() const override;

	/** 设置存储的值 (FSyInteractionListValue) */
	virtual void SetValueStruct_Implementation(const FInstancedStruct& InValue) override;

	/** 
	 * 获取交互信息列表的只读引用。
	 * @return 对内部 TArray<FInstancedStruct> 的常量引用。
	 */
	UFUNCTION(BlueprintPure, Category = "SyGameplay|Interaction Metadata")
	const TArray<FInstancedStruct>& GetInteractionList() const;

	/**
	 * 添加一个新的交互信息到列表中。
	 * @param InteractInfoInstance 包含 FSyInteractInfoBase 衍生实例的 FInstancedStruct。
	 * @return 如果添加成功返回 true。
	 */
	UFUNCTION(BlueprintCallable, Category = "SyGameplay|Interaction Metadata", meta = (AutoCreateRefTerm = "InteractInfoInstance"))
	bool AddInteraction(const FInstancedStruct& InteractInfoInstance);

	/** 
	 * 清空交互信息列表。
	 */
	UFUNCTION(BlueprintCallable, Category = "SyGameplay|Interaction Metadata")
	void ClearInteractions();

	// TODO: 可以添加更多辅助函数，如 GetInteractionByIndex, RemoveInteraction 等

protected:
	/** 存储交互信息的列表 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction Metadata", meta=(ShowTreeView, ShowCategories="Interaction Info"))
	FSyInteractionListValue InteractionListValue; // 注意: 这里直接使用了包含TArray的USTRUCT
}; 
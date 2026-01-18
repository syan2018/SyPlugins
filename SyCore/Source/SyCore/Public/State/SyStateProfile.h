#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "State/Types/StateParameterTypes.h"
#include "State/Backends/SyStateBackendBase.h"
#include "SyStateProfile.generated.h"

/**
 * USyStateProfile
 *
 * 标准化状态集合配置：
 * - DefaultInitData: 默认初始化状态
 * - BackendTypes: 该实体默认使用的后端类型集合
 */
UCLASS(BlueprintType)
class SYCORE_API USyStateProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SyState|Profile")
	FSyStateParameterSet DefaultInitData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SyState|Profile")
	TArray<TSubclassOf<USyStateBackendBase>> BackendTypes;

	/** 可选：带配置的后端模板（会在组件上复制为实例） */
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category="SyState|Profile")
	TArray<TObjectPtr<USyStateBackendBase>> BackendInstances;
};


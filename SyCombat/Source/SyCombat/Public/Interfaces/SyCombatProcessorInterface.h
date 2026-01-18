#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Types/SyCombatTypes.h"
#include "SyCombatProcessorInterface.generated.h"

UINTERFACE(BlueprintType)
class SYCOMBAT_API USyCombatProcessorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * ISyCombatProcessorInterface
 *
 * 结算链的最小处理单元（中间件模式）。
 */
class SYCOMBAT_API ISyCombatProcessorInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SyCombat|Processor")
	int32 GetProcessingPriority() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SyCombat|Processor")
	void ProcessRequest(UPARAM(ref) FSyCombatOperationRequest& InOutRequest);
};


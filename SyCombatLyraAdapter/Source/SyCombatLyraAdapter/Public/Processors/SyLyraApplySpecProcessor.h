#pragma once

#include "CoreMinimal.h"
#include "Interfaces/SyCombatProcessorInterface.h"
#include "SyLyraApplySpecProcessor.generated.h"

/**
 * USyLyraApplySpecProcessor
 *
 * 原型占位：最终把构建好的 Spec 应用到 ASC。
 */
UCLASS(Blueprintable)
class SYCOMBATLYRAADAPTER_API USyLyraApplySpecProcessor : public UObject, public ISyCombatProcessorInterface
{
	GENERATED_BODY()

public:
	virtual int32 GetProcessingPriority_Implementation() const override { return 100; }
	virtual void ProcessRequest_Implementation(FSyCombatOperationRequest& InOutRequest) override;
};


#pragma once

#include "CoreMinimal.h"
#include "Interfaces/SyCombatProcessorInterface.h"
#include "SyLyraDebugTraceProcessor.generated.h"

/**
 * USyLyraDebugTraceProcessor
 *
 * 原型占位：用于记录数值变化链路，便于调试。
 */
UCLASS(Blueprintable)
class SYCOMBATLYRAADAPTER_API USyLyraDebugTraceProcessor : public UObject, public ISyCombatProcessorInterface
{
	GENERATED_BODY()

public:
	virtual int32 GetProcessingPriority_Implementation() const override { return 0; }
	virtual void ProcessRequest_Implementation(FSyCombatOperationRequest& InOutRequest) override;
};


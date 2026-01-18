#pragma once

#include "CoreMinimal.h"
#include "Interfaces/SyCombatProcessorInterface.h"
#include "SyLyraBuildSpecProcessor.generated.h"

/**
 * USyLyraBuildSpecProcessor
 *
 * 原型占位：把 OperationRequest 转换为 GAS Spec 的准备阶段。
 * 具体实现建议在项目层根据 Lyra 规则扩展。
 */
UCLASS(Blueprintable)
class SYCOMBATLYRAADAPTER_API USyLyraBuildSpecProcessor : public UObject, public ISyCombatProcessorInterface
{
	GENERATED_BODY()

public:
	virtual int32 GetProcessingPriority_Implementation() const override { return 200; }
	virtual void ProcessRequest_Implementation(FSyCombatOperationRequest& InOutRequest) override;
};


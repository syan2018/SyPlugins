#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "SyCombatPresentationBridgeInterface.generated.h"

UINTERFACE(BlueprintType)
class SYCOMBAT_API USyCombatPresentationBridgeInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * ISyCombatPresentationBridgeInterface
 *
 * 把战斗逻辑事件转化为表现事件（镜头/特效/音效）。
 * 默认实现可基于 SyCore MessageBus 的 tag-driven 事件。
 */
class SYCOMBAT_API ISyCombatPresentationBridgeInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SyCombat|Presentation")
	void BroadcastPresentationEvent(FGameplayTag EventTag, const FVector& ContextLocation, AActor* SourceActor);
};


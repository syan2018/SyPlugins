#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Types/SyCombatTypes.h"
#include "SyCombatAbilityDriverInterface.generated.h"

UINTERFACE(BlueprintType)
class SYCOMBAT_API USyCombatAbilityDriverInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * ISyCombatAbilityDriverInterface
 *
 * 负责把 SyCombat 的 ActionRequest 转换为“具体实现”的能力驱动。
 * 典型实现：SyCombatLyraAdapter 中基于 GAS GameplayEvent 的驱动方式。
 */
class SYCOMBAT_API ISyCombatAbilityDriverInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SyCombat|Ability")
	void HandleActionRequest(const FSyCombatActionRequest& Request);
};


#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "SyCombatEntityInterface.generated.h"

UINTERFACE(BlueprintType)
class SYCOMBAT_API USyCombatEntityInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * ISyCombatEntityInterface
 *
 * 轻量“战斗实体”抽象，不依赖 GAS。
 * 实际实现通常由 `USyCombatComponent` 提供。
 */
class SYCOMBAT_API ISyCombatEntityInterface
{
	GENERATED_BODY()

public:
	/** 实体唯一ID */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SyCombat|Entity")
	FGuid GetCombatEntityId() const;

	/** 业务层可选的标签（用于筛选/判定） */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SyCombat|Entity")
	FGameplayTagContainer GetCombatTags() const;

	/** 目标点（支持骨骼/部位 Tag） */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SyCombat|Entity")
	FVector GetTargetingPoint(FGameplayTag BoneTag) const;
};


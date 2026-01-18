#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "SyCombatTypes.generated.h"

/**
 * FSyCombatActionRequest
 * - 用于输入/AI发起“想做什么”
 * - 只表达意图，不绑定具体实现（GAS/Lyra 由 Adapter 解释）
 */
USTRUCT(BlueprintType)
struct SYCOMBAT_API FSyCombatActionRequest
{
	GENERATED_BODY()

	/** 行为/技能标识（例如 Sy.Combat.Action.LightAttack） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyCombat")
	FGameplayTag ActionTag;

	/** 可选：附加参数（例如连段索引、方向、锁定信息） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyCombat")
	FInstancedStruct Payload;
};

/**
 * FSyCombatHitContext
 * - 用于“命中/判定结果”回灌（Adapter 把 GAS TargetData/HitResult 归一化为此结构）
 */
USTRUCT(BlueprintType)
struct SYCOMBAT_API FSyCombatHitContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyCombat")
	FGuid InstigatorEntityId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyCombat")
	FGuid TargetEntityId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyCombat")
	FGameplayTagContainer ContextTags;
};

/**
 * FSyCombatOperationRequest
 * - 结算链中的“可变请求体”
 * - 由 Adapter 构建/加工后最终应用（例如映射为 GE/Execution）
 */
USTRUCT(BlueprintType)
struct SYCOMBAT_API FSyCombatOperationRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyCombat")
	FGuid InstigatorEntityId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyCombat")
	FGuid TargetEntityId;

	/** 数值修改示例：Key=状态Tag，Value=数值变化 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyCombat")
	TMap<FGameplayTag, float> NumericModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyCombat")
	FGameplayTagContainer ContextTags;
};


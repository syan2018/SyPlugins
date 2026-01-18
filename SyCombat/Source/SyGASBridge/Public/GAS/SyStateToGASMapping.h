#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SyStateToGASMapping.generated.h"

class UGameplayEffect;

/**
 * SyState -> GAS Tag 映射
 *
 * 设计目标：
 * - 让任务/关卡等系统只需要写 SyState（通过 StateComponent）
 * - 由 GASBridge 统一把关键状态“变成 ASC 上的 Tag/Effect”，从而影响 Ability 可用性/分支/死亡等逻辑
 */
USTRUCT(BlueprintType)
struct SYGASBRIDGE_API FSyStateToGASTagMapping
{
	GENERATED_BODY()

	/** Sy 状态Tag（例如 Sy.State.Life.Dead 或 Sy.State.Combat.Phase.2） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sy|Mapping")
	FGameplayTag SyStateTag;

	/** 映射到 ASC 的 GameplayTag */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sy|Mapping")
	FGameplayTag GameplayTag;

	/**
	 * 是否允许客户端用 LooseTag 做即时预测反馈（例如窗口/阶段提示）
	 * - true: 客户端可添加 loose tag，服务器最终用 GE/权威状态对齐
	 * - false: 完全依赖 GE（更严格，但可能反馈更慢）
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sy|Mapping")
	bool bAllowLooseTagPrediction = true;

	/**
	 * 可选：用于“持久/复制/回滚”的 GameplayEffect（应当包含 GrantedTags=GameplayTag）
	 * - 有值：Persistent 写入默认用 GE（服务器权威）
	 * - 无值：退化为 loose tag（不建议用于需要复制一致性的关键状态）
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sy|Mapping")
	TSubclassOf<UGameplayEffect> PersistentTagEffect;
};

UCLASS(BlueprintType)
class SYGASBRIDGE_API USyStateToGASMapping : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sy|Mapping")
	TArray<FSyStateToGASTagMapping> TagMappings;
};


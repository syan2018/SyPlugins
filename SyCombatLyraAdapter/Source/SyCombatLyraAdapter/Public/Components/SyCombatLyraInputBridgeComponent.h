#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Foundation/ISyComponentInterface.h"
#include "GameplayTagContainer.h"
#include "SyCombatLyraInputBridgeComponent.generated.h"

class USyCombatComponent;
class UAbilitySystemComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogSyCombatLyraInputBridge, Log, All);

/**
 * USyCombatLyraInputBridgeComponent
 *
 * Phase 2 输入桥接组件：
 * - 监听 LyraHeroComponent::NAME_BindInputsNow 扩展事件
 * - 在 LyraInputComponent 上额外注册 AbilityAction 回调
 * - 将 InputTag 翻译为 SyCombat ActionTag
 * - 通过 USyCombatComponent::RequestAction() 发起请求
 *
 * 两种运行模式：
 * - Phase 2a（并行）：SyCombat 和 Lyra 同时工作，对比结果
 * - Phase 2b（接管）：SyCombat 接管，阻止 Lyra 原生 InputTag 通路
 */
UCLASS(Blueprintable, ClassGroup=(SyEntity), meta=(BlueprintSpawnableComponent))
class SYCOMBATLYRAADAPTER_API USyCombatLyraInputBridgeComponent : public UActorComponent, public ISyComponentInterface
{
	GENERATED_BODY()

public:
	USyCombatLyraInputBridgeComponent();

	// ISyComponentInterface
	virtual FName GetComponentType() const override { return TEXT("CombatLyraInputBridge"); }
	virtual ESyComponentInitPhase GetInitializationPhase() const override { return ESyComponentInitPhase::Functional; }
	virtual void OnSyComponentInitialized() override;

	/** InputTag -> ActionTag 映射（例如 InputTag.Ability.Fire -> Sy.Combat.Action.Fire） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SyCombat|InputBridge")
	TMap<FGameplayTag, FGameplayTag> InputTagToActionTag;

	/**
	 * 是否启用接管模式 (Phase 2b)：
	 * true  = 给 ASC 添加 Gameplay.AbilityInputBlocked 阻止 Lyra 原生 InputTag 通路
	 * false = 并行模式 (Phase 2a)，SyCombat 和 Lyra 同时工作
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyCombat|InputBridge")
	bool bTakeoverMode = false;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<USyCombatComponent> CombatComponent;

	bool bInputBound = false;
	int32 RetryCount = 0;
	static constexpr int32 MaxRetryCount = 120;

	void TryBindOrScheduleRetry();
	void BindAbilityInputToPawn(APawn* Pawn);

	void OnAbilityInputTagPressed(FGameplayTag InputTag);
	void OnAbilityInputTagReleased(FGameplayTag InputTag);
};

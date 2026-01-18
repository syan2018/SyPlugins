#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Foundation/ISyComponentInterface.h"
#include "Types/SyCombatTypes.h"
#include "SyCombatPipelineComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSyCombatActionRequested, const FSyCombatActionRequest&, Request);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSyCombatHitReported, const FSyCombatHitContext&, Hit);

/**
 * USyCombatPipelineComponent
 *
 * 管线中枢（Core Orchestration）：
 * - 接收 ActionRequest（输入/AI）
 * - 通过 Adapter（后续在 SyCombatLyraAdapter）驱动 GAS Ability 生命周期
 * - 收集 HitContext 并触发结算链（后续）
 *
 * 当前先落“骨架 + 事件”，便于快速接入 Lyra/GAS 形成闭环。
 */
UCLASS(Blueprintable, ClassGroup=(SyEntity), meta=(BlueprintSpawnableComponent))
class SYCOMBAT_API USyCombatPipelineComponent : public UActorComponent, public ISyComponentInterface
{
	GENERATED_BODY()

public:
	USyCombatPipelineComponent();

	// ISyComponentInterface
	virtual FName GetComponentType() const override { return TEXT("CombatPipeline"); }
	virtual ESyComponentInitPhase GetInitializationPhase() const override { return ESyComponentInitPhase::Functional; }
	virtual void OnSyComponentInitialized() override {}

	/** 上层系统入口：发起一个战斗动作意图 */
	UFUNCTION(BlueprintCallable, Category="SyCombat")
	void RequestAction(const FSyCombatActionRequest& Request);

	/** Adapter/技能判定回调入口：报告命中 */
	UFUNCTION(BlueprintCallable, Category="SyCombat")
	void ReportHit(const FSyCombatHitContext& Hit);

	UPROPERTY(BlueprintAssignable, Category="SyCombat|Events")
	FOnSyCombatActionRequested OnActionRequested;

	UPROPERTY(BlueprintAssignable, Category="SyCombat|Events")
	FOnSyCombatHitReported OnHitReported;
};


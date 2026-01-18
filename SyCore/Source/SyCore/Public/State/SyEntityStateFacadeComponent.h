#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Foundation/ISyComponentInterface.h"
#include "State/SyStateFacadeTypes.h"
#include "SyEntityStateFacadeComponent.generated.h"

class USyEntityComponent;
class USyStateBackendBaseComponent;

/**
 * 当 Facade 成功处理一次状态变更后广播（用于任务/AI/表现等系统监听）
 * 注意：该事件表示“Facade 接受并已提交请求”，不保证目标后端一定是立即生效（例如 GAS 可能异步复制/校正）。
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSyEntityStateChangeApplied, const FSyStateChangeRequest&, Request);

/**
 * USyEntityStateFacadeComponent
 *
 * SyCore 中“统一状态入口”的门面组件：
 * - 上层系统只依赖它进行 Query/Apply
 * - 内部按优先级调度可插拔 BackendComponents（例如 GAS 后端）
 * - 无后端时回退到现有 Generic State（USyStateComponent）与既有 StateManager（按 Type/World）
 */
UCLASS(Blueprintable, ClassGroup=(SyEntity), meta=(BlueprintSpawnableComponent))
class SYCORE_API USyEntityStateFacadeComponent : public UActorComponent, public ISyComponentInterface
{
	GENERATED_BODY()

public:
	USyEntityStateFacadeComponent();

	// ISyComponentInterface
	virtual FName GetComponentType() const override { return TEXT("StateFacade"); }
	virtual ESyComponentInitPhase GetInitializationPhase() const override { return ESyComponentInitPhase::Core; }
	virtual void OnSyComponentInitialized() override;

	/** 统一写入口：按 Scope/Layer/后端优先级路由 */
	UFUNCTION(BlueprintCallable, Category="SyState")
	bool ApplyStateChange(const FSyStateChangeRequest& Request);

	/** 统一读入口：优先从 Backend 读，后回落 Generic State 的 EffectiveState */
	UFUNCTION(BlueprintCallable, Category="SyState")
	bool TryGetEffectiveStateParam(FGameplayTag StateTag, FInstancedStruct& OutValue) const;

	template<typename T>
	bool TryGetEffectiveStateValue(FGameplayTag StateTag, T& OutValue) const
	{
		FInstancedStruct Tmp;
		if (!TryGetEffectiveStateParam(StateTag, Tmp))
		{
			return false;
		}
		if (const T* Ptr = Tmp.GetPtr<T>())
		{
			OutValue = *Ptr;
			return true;
		}
		return false;
	}

	/** 外部监听：任务/AI/表现等系统订阅此事件 */
	UPROPERTY(BlueprintAssignable, Category="SyState|Events")
	FOnSyEntityStateChangeApplied OnStateChangeApplied;

private:
	UPROPERTY(Transient)
	TObjectPtr<USyEntityComponent> EntityComponent;

	UPROPERTY(Transient)
	TArray<TObjectPtr<USyStateBackendBaseComponent>> BackendComponents;

	void RefreshBackendCache();
	bool ApplyViaBackends(const FSyStateChangeRequest& LocalRequest);
};


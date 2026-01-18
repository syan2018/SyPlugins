#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Foundation/ISyComponentInterface.h"
#include "State/SyStateFacadeTypes.h"
#include "SyStateBackendBaseComponent.generated.h"

/**
 * USyStateBackendBaseComponent
 *
 * 设计目标：
 * - 作为“可插拔后端”的统一基类：Generic / GAS / 自研Numeric 等都可以以组件形式接入
 * - 由 `USyEntityStateFacadeComponent` 统一发现、排序、调度，实现“唯一入口”
 *
 * 注意：
 * - SyCore 本身不依赖 GAS，因此 GAS 后端应定义在独立插件（例如 SyGASBridge / SyCombatLyraAdapter）中
 */
UCLASS(Abstract, Blueprintable, ClassGroup=(SyEntity), meta=(BlueprintSpawnableComponent))
class SYCORE_API USyStateBackendBaseComponent : public UActorComponent, public ISyComponentInterface
{
	GENERATED_BODY()

public:
	// ISyComponentInterface
	virtual FName GetComponentType() const override { return TEXT("StateBackend"); }
	virtual ESyComponentInitPhase GetInitializationPhase() const override { return ESyComponentInitPhase::Core; }
	virtual void OnSyComponentInitialized() override {}

	/** 越大越先被 Facade 调用 */
	UFUNCTION(BlueprintNativeEvent, Category="SyState|Backend")
	int32 GetBackendPriority() const;
	virtual int32 GetBackendPriority_Implementation() const { return 0; }

	/** 后端是否愿意处理该请求 */
	UFUNCTION(BlueprintNativeEvent, Category="SyState|Backend")
	bool CanHandleChange(const FSyStateChangeRequest& Request) const;
	virtual bool CanHandleChange_Implementation(const FSyStateChangeRequest& Request) const { return false; }

	/** 应用变更。返回 true 表示已处理并成功（Facade 会停止继续尝试） */
	UFUNCTION(BlueprintNativeEvent, Category="SyState|Backend")
	bool ApplyChange(const FSyStateChangeRequest& Request);
	virtual bool ApplyChange_Implementation(const FSyStateChangeRequest& Request) { return false; }

	/** 查询后端值（用于从 GAS/自研后端读取时覆盖 Generic 查询） */
	UFUNCTION(BlueprintNativeEvent, Category="SyState|Backend")
	bool TryGetValueStruct(const FGameplayTag& StateTag, FInstancedStruct& OutValue) const;
	virtual bool TryGetValueStruct_Implementation(const FGameplayTag& StateTag, FInstancedStruct& OutValue) const { return false; }
};


#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Foundation/ISyComponentInterface.h"
#include "State/Backends/SyStateBackendBaseComponent.h"
#include "State/SyStateFacadeTypes.h"
#include "SyGenericStateBackendComponent.generated.h"

class USyStateComponent;
class USyStateManagerSubsystem;

/**
 * USyGenericStateBackendComponent
 *
 * 作为通用状态后端（旧 State 系统的“后端化”）：
 * - Entity + Temporary -> USyStateComponent 临时层
 * - Type/World + Persistent -> USyStateManagerSubsystem（按类型记录）
 *
 * 注意：不再由 Facade 做隐式回退，必须显式挂载该组件才会生效。
 */
UCLASS(Blueprintable, ClassGroup=(SyEntity), meta=(BlueprintSpawnableComponent))
class SYCORE_API USyGenericStateBackendComponent : public USyStateBackendBaseComponent
{
	GENERATED_BODY()

public:
	USyGenericStateBackendComponent();

	virtual void OnSyComponentInitialized() override;

	// USyStateBackendBaseComponent
	virtual int32 GetBackendPriority_Implementation() const override { return 0; }
	virtual bool CanHandleChange_Implementation(const FSyStateChangeRequest& Request) const override;
	virtual bool ApplyChange_Implementation(const FSyStateChangeRequest& Request) override;
	virtual bool TryGetValueStruct_Implementation(const FGameplayTag& StateTag, FInstancedStruct& OutValue) const override;

private:
	UPROPERTY(Transient)
	TObjectPtr<USyStateComponent> StateComponent;

	UPROPERTY(Transient)
	TObjectPtr<USyStateManagerSubsystem> StateManager;
};


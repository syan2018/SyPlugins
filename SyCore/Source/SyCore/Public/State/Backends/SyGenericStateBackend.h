#pragma once

#include "CoreMinimal.h"
#include "State/Backends/SyStateBackendBase.h"
#include "SyGenericStateBackend.generated.h"

class USyStateComponent;
class USyStateManagerSubsystem;

/**
 * USyGenericStateBackend
 *
 * 作为通用状态后端（旧 State 系统的“后端化”）：
 * - Entity + Temporary -> USyStateComponent 临时层
 * - Type/World + Persistent -> USyStateManagerSubsystem（按类型记录）
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class SYCORE_API USyGenericStateBackend : public USyStateBackendBase
{
	GENERATED_BODY()

public:
	virtual void InitializeBackend(USyStateComponent* InOwner) override;

	// USyStateBackendBase
	virtual int32 GetBackendPriority_Implementation() const override { return 0; }
	virtual bool CanHandleChange_Implementation(const FSyStateChangeRequest& Request) const override;
	virtual bool ApplyChange_Implementation(const FSyStateChangeRequest& Request) override;
	virtual bool TryGetValueStruct_Implementation(const FGameplayTag& StateTag, FInstancedStruct& OutValue) const override;

private:
	UPROPERTY(Transient)
	TObjectPtr<USyStateManagerSubsystem> StateManager;
};


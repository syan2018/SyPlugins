#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "State/SyStateTypes.h"
#include "SyStateBackendBase.generated.h"

class USyStateComponent;

/**
 * USyStateBackendBase
 *
 * 统一状态后端的基类（UObject 版本）：
 * - 由 USyStateComponent 持有并初始化
 * - Generic/GAS/自研数值后端均应继承此类
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class SYCORE_API USyStateBackendBase : public UObject
{
	GENERATED_BODY()

public:
	virtual void InitializeBackend(USyStateComponent* InOwner) { OwnerStateComponent = InOwner; }

	UFUNCTION(BlueprintNativeEvent, Category="SyState|Backend")
	int32 GetBackendPriority() const;
	virtual int32 GetBackendPriority_Implementation() const { return 0; }

	UFUNCTION(BlueprintNativeEvent, Category="SyState|Backend")
	bool CanHandleChange(const FSyStateChangeRequest& Request) const;
	virtual bool CanHandleChange_Implementation(const FSyStateChangeRequest& Request) const { return false; }

	UFUNCTION(BlueprintNativeEvent, Category="SyState|Backend")
	bool ApplyChange(const FSyStateChangeRequest& Request);
	virtual bool ApplyChange_Implementation(const FSyStateChangeRequest& Request) { return false; }

	UFUNCTION(BlueprintNativeEvent, Category="SyState|Backend")
	bool TryGetValueStruct(const FGameplayTag& StateTag, FInstancedStruct& OutValue) const;
	virtual bool TryGetValueStruct_Implementation(const FGameplayTag& StateTag, FInstancedStruct& OutValue) const { return false; }

protected:
	UPROPERTY(Transient)
	TObjectPtr<USyStateComponent> OwnerStateComponent;
};


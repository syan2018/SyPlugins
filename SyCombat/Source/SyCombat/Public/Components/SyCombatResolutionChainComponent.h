#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Foundation/ISyComponentInterface.h"
#include "Interfaces/SyCombatProcessorInterface.h"
#include "SyCombatResolutionChainComponent.generated.h"

/**
 * USyCombatResolutionChainComponent
 *
 * 轻量结算链：维护 Processor 列表并按优先级执行。
 */
UCLASS(Blueprintable, ClassGroup=(SyEntity), meta=(BlueprintSpawnableComponent))
class SYCOMBAT_API USyCombatResolutionChainComponent : public UActorComponent, public ISyComponentInterface
{
	GENERATED_BODY()

public:
	USyCombatResolutionChainComponent();

	// ISyComponentInterface
	virtual FName GetComponentType() const override { return TEXT("CombatResolutionChain"); }
	virtual ESyComponentInitPhase GetInitializationPhase() const override { return ESyComponentInitPhase::Functional; }
	virtual void OnSyComponentInitialized() override {}

	UFUNCTION(BlueprintCallable, Category="SyCombat|Resolution")
	void RegisterProcessor(UObject* Processor);

	UFUNCTION(BlueprintCallable, Category="SyCombat|Resolution")
	void ClearProcessors();

	UFUNCTION(BlueprintCallable, Category="SyCombat|Resolution")
	void ExecuteChain(UPARAM(ref) FSyCombatOperationRequest& InOutRequest);

private:
	UPROPERTY()
	TArray<TScriptInterface<ISyCombatProcessorInterface>> Processors;

	void SortProcessors();
};


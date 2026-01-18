#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Foundation/ISyComponentInterface.h"
#include "Interfaces/SyCombatEntityInterface.h"
#include "Interfaces/SyCombatProcessorInterface.h"
#include "Types/SyCombatTypes.h"
#include "SyCombatComponent.generated.h"

class USyEntityComponent;

USTRUCT()
struct FSyCombatBufferedAction
{
	GENERATED_BODY()

	UPROPERTY()
	FSyCombatActionRequest Request;

	UPROPERTY()
	float ExpireAtSeconds = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSyCombatActionRequested, const FSyCombatActionRequest&, Request);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSyCombatHitReported, const FSyCombatHitContext&, Hit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSyCombatActionBuffered, const FSyCombatActionRequest&, Request);

/**
 * USyCombatComponent
 *
 * 收敛版核心组件：合并 Pipeline + InputBuffer + ResolutionChain。
 */
UCLASS(Blueprintable, ClassGroup=(SyEntity), meta=(BlueprintSpawnableComponent))
class SYCOMBAT_API USyCombatComponent : public UActorComponent, public ISyComponentInterface, public ISyCombatEntityInterface
{
	GENERATED_BODY()

public:
	USyCombatComponent();

	// ISyComponentInterface
	virtual FName GetComponentType() const override { return TEXT("Combat"); }
	virtual ESyComponentInitPhase GetInitializationPhase() const override { return ESyComponentInitPhase::Functional; }
	virtual void OnSyComponentInitialized() override;

	// 管线入口
	UFUNCTION(BlueprintCallable, Category="SyCombat")
	void RequestAction(const FSyCombatActionRequest& Request);

	UFUNCTION(BlueprintCallable, Category="SyCombat")
	void ReportHit(const FSyCombatHitContext& Hit);

	// 输入缓冲
	UFUNCTION(BlueprintCallable, Category="SyCombat|Input")
	void BufferAction(const FSyCombatActionRequest& Request);

	UFUNCTION(BlueprintCallable, Category="SyCombat|Input")
	bool ConsumeNextBufferedAction(FSyCombatActionRequest& OutRequest);

	// 结算链
	UFUNCTION(BlueprintCallable, Category="SyCombat|Resolution")
	void RegisterProcessor(UObject* Processor);

	UFUNCTION(BlueprintCallable, Category="SyCombat|Resolution")
	void ClearProcessors();

	UFUNCTION(BlueprintCallable, Category="SyCombat|Resolution")
	void ExecuteChain(UPARAM(ref) FSyCombatOperationRequest& InOutRequest);

	// ISyCombatEntityInterface
	virtual FGuid GetCombatEntityId_Implementation() const override;
	virtual FGameplayTagContainer GetCombatTags_Implementation() const override;
	virtual FVector GetTargetingPoint_Implementation(FGameplayTag BoneTag) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyCombat|Input")
	int32 MaxBufferSize = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyCombat|Input")
	float DefaultExpirationSeconds = 0.35f;

	UPROPERTY(BlueprintAssignable, Category="SyCombat|Events")
	FOnSyCombatActionRequested OnActionRequested;

	UPROPERTY(BlueprintAssignable, Category="SyCombat|Events")
	FOnSyCombatHitReported OnHitReported;

	UPROPERTY(BlueprintAssignable, Category="SyCombat|Input")
	FOnSyCombatActionBuffered OnActionBuffered;

private:
	UPROPERTY(Transient)
	TObjectPtr<USyEntityComponent> EntityComponent;

	UPROPERTY()
	TArray<FSyCombatBufferedAction> Buffer;

	UPROPERTY()
	TArray<TScriptInterface<ISyCombatProcessorInterface>> Processors;

	void CleanupExpired();
	void SortProcessors();
};


#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Foundation/ISyComponentInterface.h"
#include "Types/SyCombatTypes.h"
#include "SyCombatInputBufferComponent.generated.h"

USTRUCT()
struct FSyCombatBufferedAction
{
	GENERATED_BODY()

	UPROPERTY()
	FSyCombatActionRequest Request;

	UPROPERTY()
	float ExpireAtSeconds = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSyCombatActionBuffered, const FSyCombatActionRequest&, Request);

/**
 * USyCombatInputBufferComponent
 *
 * 轻量输入缓冲（原型）：
 * - 允许把 ActionRequest 先缓存（解决预输入/取消窗口）
 * - 由外部系统决定何时消费（例如动画 NotifyState / Ability 可取消窗口）
 */
UCLASS(Blueprintable, ClassGroup=(SyEntity), meta=(BlueprintSpawnableComponent))
class SYCOMBAT_API USyCombatInputBufferComponent : public UActorComponent, public ISyComponentInterface
{
	GENERATED_BODY()

public:
	USyCombatInputBufferComponent();

	// ISyComponentInterface
	virtual FName GetComponentType() const override { return TEXT("CombatInputBuffer"); }
	virtual ESyComponentInitPhase GetInitializationPhase() const override { return ESyComponentInitPhase::Functional; }
	virtual void OnSyComponentInitialized() override {}

	/** 缓冲一个 ActionRequest */
	UFUNCTION(BlueprintCallable, Category="SyCombat|Input")
	void BufferAction(const FSyCombatActionRequest& Request);

	/**
	 * @brief 取出下一个仍有效的 ActionRequest
	 * @return true 表示成功取出
	 */
	UFUNCTION(BlueprintCallable, Category="SyCombat|Input")
	bool ConsumeNextBufferedAction(FSyCombatActionRequest& OutRequest);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyCombat|Input")
	int32 MaxBufferSize = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyCombat|Input")
	float DefaultExpirationSeconds = 0.35f;

	UPROPERTY(BlueprintAssignable, Category="SyCombat|Input")
	FOnSyCombatActionBuffered OnActionBuffered;

private:
	UPROPERTY()
	TArray<FSyCombatBufferedAction> Buffer;

	void CleanupExpired();
};


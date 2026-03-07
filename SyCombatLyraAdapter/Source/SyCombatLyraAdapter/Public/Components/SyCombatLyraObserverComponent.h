#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Foundation/ISyComponentInterface.h"
#include "GameplayTagContainer.h"
#include "SyCombatLyraObserverComponent.generated.h"

class USyCombatComponent;
class ULyraHealthComponent;
class UAbilitySystemComponent;
class UGameplayAbility;

DECLARE_LOG_CATEGORY_EXTERN(LogSyCombatLyraObserver, Log, All);

/**
 * USyCombatLyraObserverComponent
 *
 * Phase 1 旁路监听组件：
 * - 绑定 ULyraHealthComponent 的 OnHealthChanged / OnDeathStarted
 * - 绑定 ASC 的 AbilityActivatedCallbacks
 * - 将 Lyra 战斗事件翻译为 SyCombat 日志/HitContext（纯观察，不触发结算）
 * - 验证 GASBridge State → ASC Tag 映射是否正确
 */
UCLASS(Blueprintable, ClassGroup=(SyEntity), meta=(BlueprintSpawnableComponent))
class SYCOMBATLYRAADAPTER_API USyCombatLyraObserverComponent : public UActorComponent, public ISyComponentInterface
{
	GENERATED_BODY()

public:
	USyCombatLyraObserverComponent();

	// ISyComponentInterface
	virtual FName GetComponentType() const override { return TEXT("CombatLyraObserver"); }
	virtual ESyComponentInitPhase GetInitializationPhase() const override { return ESyComponentInitPhase::Functional; }
	virtual void OnSyComponentInitialized() override;

	/** 是否将监听到的命中事件通过 ReportHit 回灌到 SyCombatComponent（默认 false，纯日志模式） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyCombat|Observer")
	bool bFeedHitsToSyCombat = false;

	/** 是否在监听到死亡时写入 SyState (Sy.State.Life.Dead)（默认 true） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyCombat|Observer")
	bool bSyncDeathToSyState = true;

	/** 死亡映射到的 SyState Tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyCombat|Observer")
	FGameplayTag DeathStateTag;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<USyCombatComponent> CombatComponent;

	UPROPERTY(Transient)
	TObjectPtr<ULyraHealthComponent> HealthComponent;

	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> ASC;

	void BindToLyraHealth();
	void BindToASC();

	UFUNCTION()
	void HandleLyraHealthChanged(ULyraHealthComponent* InHealthComp, float OldValue, float NewValue, AActor* Instigator);

	UFUNCTION()
	void HandleLyraDeathStarted(AActor* OwningActor);

	void HandleAbilityActivated(UGameplayAbility* Ability);

	void WriteSyStateDead();
};

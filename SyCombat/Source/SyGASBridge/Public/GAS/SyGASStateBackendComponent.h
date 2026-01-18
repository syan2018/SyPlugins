#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"

#include "State/Backends/SyStateBackendBaseComponent.h"

#include "SyGASStateBackendComponent.generated.h"

class UAbilitySystemComponent;
class USyStateToGASMapping;
class UGameplayEffect;

/**
 * USyGASStateBackendComponent
 *
 * 将 SyCore 的 StateFacade 写入请求映射到 GAS（ASC）上：
 * - Temporary: 默认用 LooseGameplayTags（可预测、即时）
 * - Persistent: 默认用 GameplayEffect（GrantedTags，复制/回滚友好）
 *
 * 注意：
 * - 这是“后端组件”，由 `USyEntityStateFacadeComponent` 自动发现并按优先级调用
 * - 为了最大化兼容，既支持 loose tag 也支持 effect 方式（由 Mapping 控制）
 */
UCLASS(Blueprintable, ClassGroup=(SyEntity), meta=(BlueprintSpawnableComponent))
class SYGASBRIDGE_API USyGASStateBackendComponent : public USyStateBackendBaseComponent
{
	GENERATED_BODY()

public:
	USyGASStateBackendComponent();

	virtual void OnSyComponentInitialized() override;

	// USyStateBackendBaseComponent
	virtual int32 GetBackendPriority_Implementation() const override { return 1000; }
	virtual bool CanHandleChange_Implementation(const FSyStateChangeRequest& Request) const override;
	virtual bool ApplyChange_Implementation(const FSyStateChangeRequest& Request) override;

protected:
	/** SyState -> GAS 映射表（DataAsset） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sy|GAS")
	TObjectPtr<USyStateToGASMapping> Mapping;

private:
	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> ASC;

	/** 记录由 Persistent GE 写入产生的 ActiveEffectHandle，便于移除 */
	TMap<FGameplayTag, FActiveGameplayEffectHandle> ActiveTagEffects;

	bool EnsureASC();
	const FSyStateToGASTagMapping* FindTagMapping(const FGameplayTag& SyStateTag) const;
	bool ExtractBoolValue(const FInstancedStruct& Value, bool& OutBool) const;

	bool ApplyTagAsLoose(const FGameplayTag& TagToApply, bool bEnable);
	bool ApplyTagAsEffect(const TSubclassOf<UGameplayEffect>& EffectClass, const FGameplayTag& SyStateTag, bool bEnable);
};


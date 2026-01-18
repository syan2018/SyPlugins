#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Foundation/ISyComponentInterface.h"
#include "GameplayTagContainer.h"

#include "Components/SyCombatComponent.h"
#include "Interfaces/SyCombatAbilityDriverInterface.h"

#include "SyCombatGASAbilityDriverComponent.generated.h"

class UAbilitySystemComponent;

/**
 * USyCombatGASAbilityDriverComponent
 *
 * 负责把 SyCombat 的 ActionRequest 转成 GAS 的 GameplayEvent（预测友好）：
 * - 具体 Ability 的绑定策略交给 Lyra/GAS 本身（AbilitySet + EventTag）
 * - SyCombat 仅做“流程编排”，不在 core 中依赖 GAS
 */
UCLASS(Blueprintable, ClassGroup=(SyEntity), meta=(BlueprintSpawnableComponent))
class SYCOMBATLYRAADAPTER_API USyCombatGASAbilityDriverComponent : public UActorComponent, public ISyComponentInterface, public ISyCombatAbilityDriverInterface
{
	GENERATED_BODY()

public:
	USyCombatGASAbilityDriverComponent();

	// ISyComponentInterface
	virtual FName GetComponentType() const override { return TEXT("CombatAbilityDriver"); }
	virtual ESyComponentInitPhase GetInitializationPhase() const override { return ESyComponentInitPhase::Functional; }
	virtual void OnSyComponentInitialized() override;

	/** ActionTag -> GameplayEventTag 的映射（由项目/适配层配置） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SyCombat|GAS")
	TMap<FGameplayTag, FGameplayTag> ActionToGameplayEventTag;

private:
	UPROPERTY(Transient)
	TObjectPtr<USyCombatComponent> Combat;

	UFUNCTION()
	void HandleActionRequested(const FSyCombatActionRequest& Request);

	// ISyCombatAbilityDriverInterface
	virtual void HandleActionRequest_Implementation(const FSyCombatActionRequest& Request) override;

	UAbilitySystemComponent* ResolveASC() const;
};


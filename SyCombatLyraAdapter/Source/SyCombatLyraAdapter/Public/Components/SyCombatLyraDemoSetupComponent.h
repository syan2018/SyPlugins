#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "SyCombatLyraDemoSetupComponent.generated.h"

class USyCombatComponent;
class USyCombatGASAbilityDriverComponent;

/**
 * USyCombatLyraDemoSetupComponent
 *
 * 原型演示组件：
 * - 自动挂接 SyCombat + LyraAdapter 的关键链路
 * - 注册默认 Processor
 * - 应用 ActionTag -> GameplayEventTag 的基础映射
 */
UCLASS(Blueprintable, ClassGroup=(SyEntity), meta=(BlueprintSpawnableComponent))
class SYCOMBATLYRAADAPTER_API USyCombatLyraDemoSetupComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USyCombatLyraDemoSetupComponent();

protected:
	virtual void BeginPlay() override;

public:
	/** 默认 Action -> GameplayEventTag 映射（原型使用，可在蓝图中覆盖） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SyCombat|Demo")
	TMap<FGameplayTag, FGameplayTag> DefaultActionToEvent;

private:
	void RegisterDefaultProcessors(USyCombatComponent* Combat);
	void ApplyDefaultActionMapping(USyCombatGASAbilityDriverComponent* Driver) const;
};


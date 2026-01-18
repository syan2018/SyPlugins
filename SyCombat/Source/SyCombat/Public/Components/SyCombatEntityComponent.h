#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Foundation/ISyComponentInterface.h"
#include "Interfaces/SyCombatEntityInterface.h"
#include "SyCombatEntityComponent.generated.h"

class USyEntityComponent;

/**
 * USyCombatEntityComponent
 *
 * SyCombat 视角下的“实体 Facet”，贯彻 SyEntityComponent 作为统一入口的实践：
 * - 不要求 Actor 实现某个 CombatInterface
 * - 只要挂载该组件即可被 SyCombatPipeline 识别为“可战斗实体”
 */
UCLASS(Blueprintable, ClassGroup=(SyEntity), meta=(BlueprintSpawnableComponent))
class SYCOMBAT_API USyCombatEntityComponent : public UActorComponent, public ISyComponentInterface, public ISyCombatEntityInterface
{
	GENERATED_BODY()

public:
	USyCombatEntityComponent();

	// ISyComponentInterface
	virtual FName GetComponentType() const override { return TEXT("CombatEntity"); }
	virtual ESyComponentInitPhase GetInitializationPhase() const override { return ESyComponentInitPhase::Core; }
	virtual void OnSyComponentInitialized() override;

	UFUNCTION(BlueprintPure, Category="SyCombat")
	FGuid GetEntityId() const;

	// ISyCombatEntityInterface
	virtual FGuid GetCombatEntityId_Implementation() const override;
	virtual FGameplayTagContainer GetCombatTags_Implementation() const override;

private:
	UPROPERTY(Transient)
	TObjectPtr<USyEntityComponent> EntityComponent;
};


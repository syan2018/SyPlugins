#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Foundation/ISyComponentInterface.h"
#include "Interfaces/SyCombatPresentationBridgeInterface.h"
#include "SyCombatLyraPresentationBridgeComponent.generated.h"

class UAbilitySystemComponent;

/**
 * USyCombatLyraPresentationBridgeComponent
 *
 * 示例：将表现事件转换为 GameplayCue（需要 ASC）
 */
UCLASS(Blueprintable, ClassGroup=(SyEntity), meta=(BlueprintSpawnableComponent))
class SYCOMBATLYRAADAPTER_API USyCombatLyraPresentationBridgeComponent : public UActorComponent, public ISyComponentInterface, public ISyCombatPresentationBridgeInterface
{
	GENERATED_BODY()

public:
	USyCombatLyraPresentationBridgeComponent();

	// ISyComponentInterface
	virtual FName GetComponentType() const override { return TEXT("CombatLyraPresentationBridge"); }
	virtual ESyComponentInitPhase GetInitializationPhase() const override { return ESyComponentInitPhase::Functional; }
	virtual void OnSyComponentInitialized() override {}

	// ISyCombatPresentationBridgeInterface
	virtual void BroadcastPresentationEvent_Implementation(FGameplayTag EventTag, const FVector& ContextLocation, AActor* SourceActor) override;

private:
	UAbilitySystemComponent* ResolveASC() const;
};


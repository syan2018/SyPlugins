#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Foundation/ISyComponentInterface.h"
#include "Interfaces/SyCombatPresentationBridgeInterface.h"
#include "SyCombatMessageBridgeComponent.generated.h"

class USyMessageComponent;

/**
 * USyCombatMessageBridgeComponent
 *
 * 默认表现桥接：把战斗表现事件转成 SyMessage（Tag-Driven）
 */
UCLASS(Blueprintable, ClassGroup=(SyEntity), meta=(BlueprintSpawnableComponent))
class SYCOMBAT_API USyCombatMessageBridgeComponent : public UActorComponent, public ISyComponentInterface, public ISyCombatPresentationBridgeInterface
{
	GENERATED_BODY()

public:
	USyCombatMessageBridgeComponent();

	// ISyComponentInterface
	virtual FName GetComponentType() const override { return TEXT("CombatPresentationBridge"); }
	virtual ESyComponentInitPhase GetInitializationPhase() const override { return ESyComponentInitPhase::Functional; }
	virtual void OnSyComponentInitialized() override;

	// ISyCombatPresentationBridgeInterface
	virtual void BroadcastPresentationEvent_Implementation(FGameplayTag EventTag, const FVector& ContextLocation, AActor* SourceActor) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<USyMessageComponent> MessageComponent;
};


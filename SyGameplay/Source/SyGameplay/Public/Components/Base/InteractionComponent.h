/**
 * @brief 参考自 FlowSolo 项目
 * @see https://github.com/MothCocoon/FlowSolo
 */

#pragma once

#include "Components/ArrowComponent.h"
#include "InteractionComponent.generated.h"

class APlayerCameraManager;

DECLARE_MULTICAST_DELEGATE_OneParam(FPlayerInInteractionEvent, TWeakObjectPtr<class UInteractionComponent> /*Interaction*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInteractionComponentEvent);

/**
 * Interaction Component
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class SYGAMEPLAY_API UInteractionComponent : public UArrowComponent
{
	GENERATED_UCLASS_BODY()

	static FPlayerInInteractionEvent OnPlayerEnter;
	static FPlayerInInteractionEvent OnPlayerExit;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bEnabled;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = 50.0f))
	float Distance;

private:
	bool bCanInteract;
	TWeakObjectPtr<APlayerCameraManager> CameraManager;
	
public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Enable();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Disable();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FInteractionComponentEvent OnUsed;
};

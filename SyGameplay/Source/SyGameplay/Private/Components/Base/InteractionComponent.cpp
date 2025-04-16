/**
 * @brief 参考自 FlowSolo 项目
 * @see https://github.com/MothCocoon/FlowSolo
 */

#include "Components/Base/InteractionComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"

/**	当前通过自主Tick检测PlayerEnter，并基于此向InteractionManager更新自身存在
 *	TODO: 更新为基于玩家组件自主扫描Tick，自身维护ConditionMet 
 *	在条件满足时，由玩家组件提交予InteractionManager
 */
FPlayerInInteractionEvent UInteractionComponent::OnPlayerEnter;
FPlayerInInteractionEvent UInteractionComponent::OnPlayerExit;

UInteractionComponent::UInteractionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Distance(100.0f)
{
	bAutoActivate = true;

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetUsingAbsoluteScale(true);
	ArrowColor = FColor::Red;
	ArrowSize = 0.5f;
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bEnabled)
	{
		Enable();
	}
}

void UInteractionComponent::Enable()
{
	if (const APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		bEnabled = true;

		PrimaryComponentTick.SetTickFunctionEnable(true);
	}
}

void UInteractionComponent::Disable()
{
	if (bCanInteract)
	{
		bCanInteract = false;
		OnPlayerExit.Broadcast(this);
	}

	bEnabled = false;

	PrimaryComponentTick.SetTickFunctionEnable(false);
}

bool UInteractionComponent::CheckConditionMet() const
{
	bool bConditionsMet = false;
	if (const APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		const FVector DistanceToActor = GetComponentLocation() - PlayerController->GetPawn()->GetActorLocation();
		bConditionsMet = DistanceToActor.Size() < Distance;
		
	}
	return bConditionsMet;
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bool bConditionsMet = CheckConditionMet())
	{
		if (!bCanInteract)
		{
			bCanInteract = true;
			OnPlayerEnter.Broadcast(this);
		}
	}
	else if (bCanInteract)
	{
		bCanInteract = false;
		OnPlayerExit.Broadcast(this);
	}
}

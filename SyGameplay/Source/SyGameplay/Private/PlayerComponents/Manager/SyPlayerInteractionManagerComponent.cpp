#include "PlayerComponents/Manager/SyPlayerInteractionManagerComponent.h"
#include "Components/SyInteractionComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

USyPlayerInteractionManagerComponent::USyPlayerInteractionManagerComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // 注意：这里不再创建DetectionSphere
}

void USyPlayerInteractionManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    // TODO: 未来应移除以下静态事件订阅，改为由 InteractionDetectorComponent 驱动
    // 订阅交互组件的静态进入/离开事件
    USyInteractionComponent::OnPlayerEnter.AddUObject(this, &USyPlayerInteractionManagerComponent::HandleInteractionEnter);
    USyInteractionComponent::OnPlayerExit.AddUObject(this, &USyPlayerInteractionManagerComponent::HandleInteractionExit);
    // --- End of TODO section ---
}

void USyPlayerInteractionManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // TODO: 未来应移除以下静态事件解绑
    // 解绑静态事件
    USyInteractionComponent::OnPlayerEnter.RemoveAll(this);
    USyInteractionComponent::OnPlayerExit.RemoveAll(this);
    // --- End of TODO section ---

    Super::EndPlay(EndPlayReason);
}

void USyPlayerInteractionManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 更新激活的交互组件
    UpdateActiveInteraction(DeltaTime);
}

// --- TODO Section: Static Event Handlers (To be removed later) ---
void USyPlayerInteractionManagerComponent::HandleInteractionEnter(const TWeakObjectPtr<UInteractionComponent> InteractionComponent)
{
    USyInteractionComponent* SyInteractionComponent = Cast<USyInteractionComponent>(InteractionComponent.Get());
    if (SyInteractionComponent && SyInteractionComponent->bEnabled)
    {
        PotentialInteractions.AddUnique(SyInteractionComponent);
    }
}

void USyPlayerInteractionManagerComponent::HandleInteractionExit(const TWeakObjectPtr<UInteractionComponent> InteractionComponent)
{
    USyInteractionComponent* SyInteractionComponent = Cast<USyInteractionComponent>(InteractionComponent.Get());
    if (SyInteractionComponent && SyInteractionComponent->bEnabled)
    {
        PotentialInteractions.Remove(SyInteractionComponent);
        // 如果离开的是当前激活的交互物，则立即清空激活状态
        if (ActiveInteraction == InteractionComponent)
        {
            SetActiveInteraction(nullptr);
        }
    }
}
// --- End of TODO Section ---

void USyPlayerInteractionManagerComponent::UpdateActiveInteraction(float DeltaTime)
{
    // 清理无效的潜在交互物 (可能已被销毁)
    PotentialInteractions.RemoveAll([](const TWeakObjectPtr<USyInteractionComponent>& Ptr){ return !Ptr.IsValid(); });

    USyInteractionComponent* ClosestInteraction = nullptr;
    float MinDistSq = TNumericLimits<float>::Max();

    // 检查 Owner 是否有效
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        if (ActiveInteraction.IsValid())
        {
            SetActiveInteraction(nullptr); // 如果Owner无效，清除激活的交互
        }
        return;
    }
    const FVector OwnerLocation = OwnerActor->GetActorLocation();

    // 遍历所有潜在交互物，找到最近且可交互的那个
    for (const auto& InteractionPtr : PotentialInteractions)
    {
        // 再次检查 InteractionPtr 和其 Owner 是否有效
        if (InteractionPtr.IsValid() && InteractionPtr->GetOwner() && InteractionPtr->bEnabled)
        {
            const float DistSq = FVector::DistSquared(OwnerLocation, InteractionPtr->GetOwner()->GetActorLocation());
            if (DistSq < MinDistSq)
            {
                MinDistSq = DistSq;
                ClosestInteraction = InteractionPtr.Get();
            }
        }
    }

    // 如果找到的最近交互物与当前激活的不同，则更新
    if (ActiveInteraction.Get() != ClosestInteraction)
    {
        SetActiveInteraction(ClosestInteraction);
    }
}

void USyPlayerInteractionManagerComponent::SetActiveInteraction(USyInteractionComponent* NewInteraction)
{
    // 检查是否真的发生了变化
    if (ActiveInteraction.Get() != NewInteraction)
    {
        ActiveInteraction = NewInteraction;
        // 广播事件，通知UI或其他系统
        OnActiveInteractionChanged.Broadcast(NewInteraction);

        // UE_LOG(LogTemp, Log, TEXT("Active Interaction Changed: %s"), *GetNameSafe(NewInteraction ? NewInteraction->GetOwner() : nullptr));
    }
}

bool USyPlayerInteractionManagerComponent::TryExecuteActiveInteraction()
{
    if (ActiveInteraction.IsValid() && ActiveInteraction->bEnabled)
    {
        // 直接广播交互组件自己的 OnUsed 委托
        ActiveInteraction->OnUsed.Broadcast();
        // UE_LOG(LogTemp, Log, TEXT("Executed interaction: %s"), *GetNameSafe(ActiveInteraction->GetOwner()));
        return true;
    }
    return false;
}

USyInteractionComponent* USyPlayerInteractionManagerComponent::GetActiveInteraction() const
{
    return ActiveInteraction.Get();
}

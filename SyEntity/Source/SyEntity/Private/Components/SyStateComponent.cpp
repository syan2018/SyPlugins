#include "Components/SyStateComponent.h"
#include "Components/SyEntityComponent.h"
#include "States/SyStateManager.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"

USyStateComponent::USyStateComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bEnableGlobalSyncByDefault = true;
}

void USyStateComponent::InitializeComponent()
{
    Super::InitializeComponent();
    
    // 获取OwnerEntityComponent引用
    AActor* Owner = GetOwner();
    if (Owner)
    {
        OwnerEntityComponent = Owner->FindComponentByClass<USyEntityComponent>();
    }
    
    // 获取StateManager引用
    if (GetWorld())
    {
        StateManager = GetWorld()->GetSubsystem<USyStateManager>();
    }
    
    // 应用InitialStates到CurrentStates
    for (const auto& Pair : InitialStates)
    {
        CurrentStates.Add(Pair.Key, Pair.Value);
    }
}

void USyStateComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // 如果启用了全局同步，可能需要从StateManager拉取状态
    if (bEnableGlobalSyncByDefault && StateManager)
    {
        PullStateFromManager();
    }
}

bool USyStateComponent::GetState(const FGameplayTag& StateTag, bool bDefaultValue) const
{
    const bool* Value = CurrentStates.Find(StateTag);
    if (Value)
    {
        return *Value;
    }
    return bDefaultValue;
}

void USyStateComponent::SetState(const FGameplayTag& StateTag, bool bNewValue, bool bNotify, bool bSyncGlobal)
{
    // 检查状态是否已存在且值相同
    bool* ExistingValue = CurrentStates.Find(StateTag);
    if (ExistingValue && *ExistingValue == bNewValue)
    {
        return; // 状态未变化，无需更新
    }
    
    // 更新状态
    CurrentStates.Add(StateTag, bNewValue);
    
    // 如果需要通知，则广播事件
    if (bNotify)
    {
        InternalOnStateChanged.Broadcast(StateTag, bNewValue);
    }
    
    // 如果需要同步到全局，则更新StateManager
    if (bSyncGlobal && bEnableGlobalSyncByDefault && StateManager)
    {
        StateManager->UpdateEntityState(GetOwnerEntityId(), StateTag, bNewValue);
    }
}

bool USyStateComponent::HasState(const FGameplayTag& StateTag) const
{
    return CurrentStates.Contains(StateTag);
}

void USyStateComponent::PullStateFromManager()
{
    if (!StateManager)
    {
        return;
    }
    
    FGuid EntityId = GetOwnerEntityId();
    if (!EntityId.IsValid())
    {
        return;
    }
    
    bool bFound = false;
    TMap<FGameplayTag, bool> GlobalStates = StateManager->GetAllEntityStates(EntityId, bFound);
    
    if (bFound)
    {
        // 更新本地状态
        for (const auto& Pair : GlobalStates)
        {
            CurrentStates.Add(Pair.Key, Pair.Value);
        }
    }
}

void USyStateComponent::PushStateToManager()
{
    if (!StateManager)
    {
        return;
    }
    
    FGuid EntityId = GetOwnerEntityId();
    if (!EntityId.IsValid())
    {
        return;
    }
    
    // 将本地所有状态推送到StateManager
    for (const auto& Pair : CurrentStates)
    {
        StateManager->UpdateEntityState(EntityId, Pair.Key, Pair.Value);
    }
}

FGuid USyStateComponent::GetOwnerEntityId() const
{
    if (OwnerEntityComponent)
    {
        return OwnerEntityComponent->GetEntityId();
    }
    return FGuid();
} 
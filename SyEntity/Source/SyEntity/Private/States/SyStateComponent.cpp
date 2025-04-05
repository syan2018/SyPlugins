#include "States/SyStateComponent.h"
#include "Components/SyEntityComponent.h"
#include "States/SyStateManager.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

USyStateComponent::USyStateComponent()
{
    // 设置组件属性
    PrimaryComponentTick.bCanEverTick = false;
    bEnableGlobalSyncByDefault = true;
}

void USyStateComponent::InitializeComponent()
{
    Super::InitializeComponent();
    
    // 获取Owner EntityComponent引用
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
    if (!StateTag.IsValid())
    {
        return bDefaultValue;
    }
    
    const bool* Value = CurrentStates.Find(StateTag);
    if (Value)
    {
        return *Value;
    }
    
    return bDefaultValue;
}

void USyStateComponent::SetState(const FGameplayTag& StateTag, bool bNewValue, bool bSyncGlobal)
{
    if (!StateTag.IsValid())
    {
        return;
    }
    
    // 检查状态是否已存在且值相同
    bool* ExistingValue = CurrentStates.Find(StateTag);
    if (ExistingValue && *ExistingValue == bNewValue)
    {
        return; // 状态未变化，无需更新
    }
    
    // 更新状态
    CurrentStates.Add(StateTag, bNewValue);
    
    // 广播内部状态变更事件
    InternalOnStateChanged.Broadcast(StateTag, bNewValue);
    
    // 如果启用了全局同步，则同步到StateManager
    if (bSyncGlobal && bEnableGlobalSyncByDefault && StateManager)
    {
        FGuid EntityId = GetOwnerEntityId();
        if (EntityId.IsValid())
        {
            StateManager->UpdateEntityState(EntityId, StateTag, bNewValue);
        }
    }
}

bool USyStateComponent::HasState(const FGameplayTag& StateTag) const
{
    if (!StateTag.IsValid())
    {
        return false;
    }
    
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
    
    // 将所有本地状态推送到StateManager
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
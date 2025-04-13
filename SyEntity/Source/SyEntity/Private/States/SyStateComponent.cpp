// Copyright Epic Games, Inc. All Rights Reserved.

#include "States/SyStateComponent.h"
#include "SyStateManager/Public/SyStateManagerSubsystem.h" // 包含 StateManager 子系统
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogSyStateComponent, Log, All); // 添加日志分类

USyStateComponent::USyStateComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    // TargetTypeTag 可以在蓝图或派生类中设置默认值
}

void USyStateComponent::BeginPlay()
{
    Super::BeginPlay();

    // TODO: 优化初始化时序，交由EntityComponent进行统一激活 
    // 1. 应用默认初始化数据
    ApplyInitializationData(DefaultInitData);

    // 2. 如果启用全局同步，则尝试连接到 StateManager
    if (bEnableGlobalSync)
    {
        TryConnectToStateManager();

        // 3. 从历史记录同步初始状态 (仅在启用同步时有意义)
        if (StateManagerSubsystem)
        {
            ApplyAggregatedModifications();
        }
        else
        {
            UE_LOG(LogSyStateComponent, Warning, TEXT("%s: Could not connect to StateManagerSubsystem on BeginPlay to sync initial state."), *GetNameSafe(GetOwner()));
        }
    }
    else
    {
        UE_LOG(LogSyStateComponent, Log, TEXT("%s: Global sync is disabled for this component."), *GetNameSafe(GetOwner()));
    }
}

void USyStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 断开与 StateManager 的连接
    DisconnectFromStateManager();

    Super::EndPlay(EndPlayReason);
}

void USyStateComponent::ApplyInitializationData(const FSyStateParameterSet& InitData)
{
    // 调用 FSyStateCategories 的方法来应用初始化数据
    // 注意：这会清空并基于 InitData 重建状态，还是合并？取决于 ApplyInitData 的实现
    CurrentStateCategories.ApplyInitData(InitData);
    UE_LOG(LogSyStateComponent, Verbose, TEXT("%s: Applied initialization data."), *GetNameSafe(GetOwner()));

    // 触发本地状态变更事件
    OnLocalStateDataChanged.Broadcast();
}

void USyStateComponent::SetTargetTypeTag(const FGameplayTag& NewTag)
{
    TargetTypeTag = NewTag;
    // TODO: [拓展] 如果已经连接到 StateManager，可能需要重新评估或触发一次同步？
    // 这取决于是否希望 TargetTag 动态改变后立即反应历史记录。
    // ApplyAggregatedModifications();
}


void USyStateComponent::TryConnectToStateManager()
{
    if (!GetOwner() || !GetWorld())
    {
        return;
    }

    // 防止重复连接 (检查 StateManagerSubsystem 是否已存在，且可能需要一个bool标志位判断是否已绑定)
    // 但由于 AddDynamic 内部通常会处理重复添加，这里主要检查 StateManagerSubsystem 是否有效即可
    if (StateManagerSubsystem)
    {
        // 如果已经获取过子系统，则可能已经绑定，不再重复操作
        // 除非有特殊需求需要强制重新绑定
        return; 
    }

    UGameInstance* GameInstance = GetWorld()->GetGameInstance();
    if (!GameInstance)
    {
        UE_LOG(LogSyStateComponent, Warning, TEXT("%s: Failed to get GameInstance."), *GetNameSafe(GetOwner()));
        return;
    }

    StateManagerSubsystem = GameInstance->GetSubsystem<USyStateManagerSubsystem>();

    if (StateManagerSubsystem)
    {
        // 订阅 StateManager 的事件 (使用 AddDynamic)
        StateManagerSubsystem->OnStateModificationRecorded.AddDynamic(this, &USyStateComponent::HandleStateModificationRecorded);
        UE_LOG(LogSyStateComponent, Log, TEXT("%s: Connected to StateManagerSubsystem and subscribed to events."), *GetNameSafe(GetOwner()));
    }
    else
    {
        UE_LOG(LogSyStateComponent, Warning, TEXT("%s: Failed to get StateManagerSubsystem."), *GetNameSafe(GetOwner()));
    }
}

void USyStateComponent::DisconnectFromStateManager()
{
    // 仅当 StateManagerSubsystem 有效时才尝试解绑
    if (StateManagerSubsystem)
    {
        // 解绑 StateManager 的事件 (使用 RemoveDynamic)
        StateManagerSubsystem->OnStateModificationRecorded.RemoveDynamic(this, &USyStateComponent::HandleStateModificationRecorded);
        UE_LOG(LogSyStateComponent, Log, TEXT("%s: Disconnected from StateManagerSubsystem."), *GetNameSafe(GetOwner()));
    }
    // 不需要手动将 StateManagerSubsystem 设为 nullptr，因为它是 TObjectPtr，会自动处理
}

void USyStateComponent::HandleStateModificationRecorded(const FSyStateModificationRecord& NewRecord)
{
    if (!StateManagerSubsystem)
    {
        UE_LOG(LogSyStateComponent, Warning, TEXT("%s: Received state record but StateManagerSubsystem is null."), *GetNameSafe(GetOwner()));
        return;
    }

    // 检查记录的目标类型是否与本组件匹配
    if (TargetTypeTag.IsValid() && NewRecord.Operation.Target.TargetTypeTag.MatchesTag(TargetTypeTag)) // 使用 MatchesTag 可能更灵活
    {
        // TODO: [拓展] 在这里可以添加更复杂的过滤逻辑，例如检查 EntityId (如果实现了的话)
        // if (NewRecord.Operation.Target.TargetEntityId != this->EntityId) { return; }

        UE_LOG(LogSyStateComponent, Verbose, TEXT("%s: Relevant state record received (OpID: %s). Applying aggregated modifications."), 
            *GetNameSafe(GetOwner()), *NewRecord.Operation.OperationId.ToString());
        
        // 重新获取并应用聚合后的状态
        ApplyAggregatedModifications();
    }
    // else: 如果 TargetTypeTag 无效，或者不匹配，则忽略此记录
}

void USyStateComponent::ApplyAggregatedModifications()
{
    if (!StateManagerSubsystem)
    {
        UE_LOG(LogSyStateComponent, Warning, TEXT("%s: Cannot apply aggregated modifications, StateManagerSubsystem is null."), *GetNameSafe(GetOwner()));
        return;
    }
    if (!TargetTypeTag.IsValid())
    {
        UE_LOG(LogSyStateComponent, Warning, TEXT("%s: Cannot apply aggregated modifications, TargetTypeTag is invalid."), *GetNameSafe(GetOwner()));
        return; // 如果没有有效的 TargetTag，无法获取聚合状态
    }

    // 从 StateManager 获取聚合后的修改
    FSyStateParameterSet AggregatedMods = StateManagerSubsystem->GetAggregatedModifications(TargetTypeTag);

    // 调用 FSyStateCategories 的方法来应用修改
    // 注意：这里需要将 FSyStateParameterSet 的内容转换为 ApplyStateModifications 需要的 TMap<FGameplayTag, FSyStateParams>
    // 如果 ApplyStateModifications 的实现是智能合并/覆盖，则此方法有效。
    CurrentStateCategories.ApplyStateModifications(AggregatedMods.GetParametersAsMap()); 
    
    UE_LOG(LogSyStateComponent, Verbose, TEXT("%s: Applied aggregated modifications for TargetTag %s."), *GetNameSafe(GetOwner()), *TargetTypeTag.ToString());

    // 触发本地状态变更事件
    OnLocalStateDataChanged.Broadcast();
}


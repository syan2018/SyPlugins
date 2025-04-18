// Copyright Epic Games, Inc. All Rights Reserved.

#include "States/SyStateComponent.h"
#include "SyStateManager/Public/SyStateManagerSubsystem.h" // 包含 StateManager 子系统
#include "SyEntityComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogSyStateComponent, Log, All); // 添加日志分类

USyStateComponent::USyStateComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

// TODO: 优化初始化时序，交由EntityComponent进行统一激活 
void USyStateComponent::BeginPlay()
{
    Super::BeginPlay();

    // 查找并缓存EntityComponent
    FindAndCacheEntityComponent();

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

void USyStateComponent::FindAndCacheEntityComponent()
{
    if (!GetOwner())
    {
        return;
    }

    // 查找EntityComponent
    EntityComponent = GetOwner()->FindComponentByClass<USyEntityComponent>();
    if (!EntityComponent)
    {
        UE_LOG(LogSyStateComponent, Warning, TEXT("%s: Could not find EntityComponent on owner actor."), *GetNameSafe(GetOwner()));
    }
}

FGameplayTag USyStateComponent::GetTargetTypeTag() const
{
    if (!EntityComponent)
    {
        return FGameplayTag();
    }

    // 获取EntityComponent的所有Tags
    FGameplayTagContainer EntityTags = EntityComponent->GetEntityTags();
    
    // 返回第一个Tag作为目标类型标签
    if (EntityTags.Num() > 0)
    {
        return EntityTags.First();
    }

    return FGameplayTag();
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
        StateManagerSubsystem->OnStateModificationChanged.AddDynamic(this, &USyStateComponent::HandleStateModificationRecorded);
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
        StateManagerSubsystem->OnStateModificationChanged.RemoveDynamic(this, &USyStateComponent::HandleStateModificationRecorded);
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

    // 获取当前的目标类型标签
    FGameplayTag CurrentTargetTag = GetTargetTypeTag();
    if (!CurrentTargetTag.IsValid())
    {
        return; // 如果没有有效的目标标签，则忽略所有记录
    }

    // TODO: 消息分发应该由Manager索引和优化，避免边缘处理 
    
    // 检查记录的目标类型是否与本组件匹配
    if (NewRecord.Operation.Target.TargetTypeTag.MatchesTag(CurrentTargetTag))
    {
        UE_LOG(LogSyStateComponent, Verbose, TEXT("%s: Relevant state record received (OpID: %s). Applying aggregated modifications."), 
            *GetNameSafe(GetOwner()), *NewRecord.Operation.OperationId.ToString());
        
        // 重新获取并应用聚合后的状态
        ApplyAggregatedModifications();
    }
}

void USyStateComponent::ApplyAggregatedModifications()
{
    if (!StateManagerSubsystem)
    {
        UE_LOG(LogSyStateComponent, Warning, TEXT("%s: Cannot apply aggregated modifications, StateManagerSubsystem is null."), *GetNameSafe(GetOwner()));
        return;
    }

    // 获取当前的目标类型标签
    FGameplayTag CurrentTargetTag = GetTargetTypeTag();
    if (!CurrentTargetTag.IsValid())
    {
        UE_LOG(LogSyStateComponent, Warning, TEXT("%s: Cannot apply aggregated modifications, TargetTypeTag is invalid."), *GetNameSafe(GetOwner()));
        return;
    }

    // 从 StateManager 获取聚合后的修改
    FSyStateParameterSet AggregatedMods = StateManagerSubsystem->GetAggregatedModifications(CurrentTargetTag);

    // 调用 FSyStateCategories 的方法来应用修改
    CurrentStateCategories.ApplyStateModifications(AggregatedMods.GetParametersAsMap()); 
    
    UE_LOG(LogSyStateComponent, Verbose, TEXT("%s: Applied aggregated modifications for TargetTag %s."), *GetNameSafe(GetOwner()), *CurrentTargetTag.ToString());

    // 触发本地状态变更事件
    OnLocalStateDataChanged.Broadcast();
}


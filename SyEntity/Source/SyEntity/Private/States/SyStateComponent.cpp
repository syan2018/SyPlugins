// Copyright Epic Games, Inc. All Rights Reserved.

#include "States/SyStateComponent.h"
#include "SyStateManager/Public/SyStateManagerSubsystem.h" // 包含 StateManager 子系统
#include "SyEntityComponent.h" // Include Entity Component
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"
#include "StateContainerTypes.h" // Included via header, but good practice
#include "StateParameterTypes.h" // Included via header, but good practice
#include "Engine/GameInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogSyStateComponent, Log, All); // 添加日志分类

USyStateComponent::USyStateComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

// TODO: （加急！！！不然交互初始化时序会爆炸）优化初始化时序，交由EntityComponent进行统一激活 
void USyStateComponent::BeginPlay()
{
    Super::BeginPlay();

    // 查找并缓存EntityComponent
    FindAndCacheEntityComponent();

    // Apply default init data to LocalState ONLY
    ApplyInitializationData(DefaultInitData); 

    if (bEnableGlobalSync)
    {
        TryConnectToStateManager();
        if (StateManagerSubsystem)
        {
            ApplyAggregatedModifications(); // Apply initial global state
        }
        else { UE_LOG(LogSyStateComponent, Warning, TEXT("%s: Could not connect to StateManagerSubsystem on BeginPlay."), *GetNameSafe(GetOwner())); }
    }
    else { UE_LOG(LogSyStateComponent, Log, TEXT("%s: Global sync is disabled."), *GetNameSafe(GetOwner())); }
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
    UE_LOG(LogSyStateComponent, Log, TEXT("%s: Applying initialization data to Default layer."), *GetNameSafe(GetOwner()));
    
    // 应用到默认层
    LayeredState.ApplyParameterSetToLayer(ESyStateLayer::Default, InitData);

    // Broadcast that the effective state has changed
    OnEffectiveStateChanged.Broadcast();
}

void USyStateComponent::ApplyTemporaryModifications(const FSyStateParameterSet& TempModifications)
{
    UE_LOG(LogSyStateComponent, Log, TEXT("%s: Applying temporary modifications to Temporary layer."), *GetNameSafe(GetOwner()));
    
    // 应用到临时层
    LayeredState.ApplyParameterSetToLayer(ESyStateLayer::Temporary, TempModifications);

    // Broadcast that the effective state has changed
    OnEffectiveStateChanged.Broadcast();
}

void USyStateComponent::ClearStateLayer(ESyStateLayer Layer)
{
    UE_LOG(LogSyStateComponent, Log, TEXT("%s: Clearing layer %d."), *GetNameSafe(GetOwner()), (int32)Layer);
    
    LayeredState.ClearLayer(Layer);

    // Broadcast that the effective state has changed
    OnEffectiveStateChanged.Broadcast();
}

// --- State Access ---

const FSyStateCategories& USyStateComponent::GetStateLayer(ESyStateLayer Layer) const
{
    return LayeredState.GetLayer(Layer);
}

FSyStateCategories USyStateComponent::GetEffectiveStateCategories() const
{
    // 使用分层容器的缓存机制获取有效状态
    return LayeredState.GetEffectiveState();
}

bool USyStateComponent::GetEffectiveStateParam(FGameplayTag StateTag, FInstancedStruct& OutParam) const
{
    // 获取有效状态（已自动按优先级合并）
    FSyStateCategories EffectiveState = GetEffectiveStateCategories();
    
    if (const FSyStateMetadatas* Metadatas = EffectiveState.GetStateDataMap().Find(StateTag))
    {
        // Find the first valid metadata param
        for(const auto& MetaPtr : Metadatas->MetadataArray)
        {
            if(const USyStateMetadataBase* Metadata = Cast<USyStateMetadataBase>(MetaPtr))
            {
                 OutParam = Metadata->GetValueStruct();
                 if (OutParam.IsValid()) return true;
            }
        }
    }

    // Not found
    OutParam.Reset();
    return false;
}

// --- Internal Sync Logic --- 

void USyStateComponent::TryConnectToStateManager()
{
    if (StateManagerSubsystem || !GetWorld()) return; // Already connected or no world

    UGameInstance* GameInstance = GetWorld()->GetGameInstance();
    if (!GameInstance) return;

    StateManagerSubsystem = GameInstance->GetSubsystem<USyStateManagerSubsystem>();
    if (StateManagerSubsystem)
    {
        FGameplayTag TargetTag = GetTargetTypeTag();
        if (!TargetTag.IsValid())
        {
            UE_LOG(LogSyStateComponent, Error, TEXT("%s: Cannot subscribe to StateManager - TargetTag is invalid. Entity needs valid tags!"), 
                *GetNameSafe(GetOwner()));
            return;
        }
        
        // 使用智能订阅（只订阅相关的目标类型）
        FOnStateModificationChangedNative Delegate;
        Delegate.BindUObject(this, &USyStateComponent::HandleStateModificationChanged);
        
        StateManagerSubsystem->SubscribeToTargetType(TargetTag, this, Delegate);
        UE_LOG(LogSyStateComponent, Log, TEXT("%s: ✅ Subscribed to StateManager for target type: %s"), 
            *GetNameSafe(GetOwner()), *TargetTag.ToString());
    }
    else 
    { 
        UE_LOG(LogSyStateComponent, Error, TEXT("%s: Failed to get StateManagerSubsystem."), *GetNameSafe(GetOwner())); 
    }
}

void USyStateComponent::DisconnectFromStateManager()
{
    if (StateManagerSubsystem)
    {
        // 取消所有智能订阅
        StateManagerSubsystem->UnsubscribeAll(this);
        
        UE_LOG(LogSyStateComponent, Log, TEXT("%s: 🔌 Disconnected from StateManagerSubsystem."), *GetNameSafe(GetOwner()));
    }
}

void USyStateComponent::HandleStateModificationChanged(const FSyStateModificationRecord& ChangedRecord)
{
    if (!StateManagerSubsystem || !bEnableGlobalSync) return;

    // 智能订阅已过滤不相关记录，直接应用
    UE_LOG(LogSyStateComponent, VeryVerbose, TEXT("%s: 📨 Received state modification (OpID: %s). Re-applying aggregated modifications."),
        *GetNameSafe(GetOwner()), *ChangedRecord.Operation.OperationId.ToString());
     
    ApplyAggregatedModifications();
}

void USyStateComponent::ApplyAggregatedModifications()
{
    if (!StateManagerSubsystem || !bEnableGlobalSync) return;

    FGameplayTag CurrentTargetTag = GetTargetTypeTag();
    if (!CurrentTargetTag.IsValid()) 
    { 
        UE_LOG(LogSyStateComponent, Warning, TEXT("%s: Cannot apply mods, invalid TargetTag."), *GetNameSafe(GetOwner())); 
        return; 
    }

    FSyStateParameterSet AggregatedMods = StateManagerSubsystem->GetAggregatedModifications(CurrentTargetTag);

    // 应用到持久层（全局状态层）
    LayeredState.ApplyParameterSetToLayer(ESyStateLayer::Persistent, AggregatedMods);

    UE_LOG(LogSyStateComponent, Verbose, TEXT("%s: Applied aggregated modifications to Persistent layer for Tag %s."), 
        *GetNameSafe(GetOwner()), *CurrentTargetTag.ToString());

    // Broadcast that the effective state definitely changed
    OnEffectiveStateChanged.Broadcast();
}


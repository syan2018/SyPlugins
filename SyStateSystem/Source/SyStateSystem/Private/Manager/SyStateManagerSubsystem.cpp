// Copyright Epic Games, Inc. All Rights Reserved.

#include "Manager/SyStateManagerSubsystem.h"
#include "Operations/OperationTypes.h" // 需要 FSyOperation 定义
#include "Core/StateParameterTypes.h" // 需要 FSyStateParameterSet 定义
#include "GameplayTagContainer.h" // 需要 FGameplayTag 定义
#include "Logging/LogMacros.h" // 用于日志输出
#include "Runtime/Launch/Resources/Version.h" // 用于 ENGINE_MAJOR_VERSION 等宏
#include "Manager/SyStateManagerSaveGame.h" // 包含自定义 SaveGame 类
#include "Kismet/GameplayStatics.h" // 包含 GameplayStatics
#include "StructUtils/InstancedStruct.h"
#include "Core/Metadatas/ListMetadataValueTypes.h" // *** 包含新的列表基类头文件 ***
#include "Algo/RemoveIf.h" // Needed for RemoveAll Swap

// 定义一个简单的日志分类
DEFINE_LOG_CATEGORY_STATIC(LogSyStateManager, Log, All); // 启用日志以方便调试

void USyStateManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogSyStateManager, Log, TEXT("SyStateManagerSubsystem Initialized."));
    // 在子系统初始化时尝试加载存档
    // TODO: 接入正常读档逻辑
    // LoadLog();
}

void USyStateManagerSubsystem::Deinitialize()
{
    UE_LOG(LogSyStateManager, Log, TEXT("SyStateManagerSubsystem Deinitializing."));
    // 在子系统反初始化前尝试保存日志（确保游戏退出时也能保存）
    // TODO: 接入正常读档逻辑
    // SaveLog();
    ModificationLog.Empty();
    OnStateModificationChanged.Clear(); // Clear the unified delegate
    Super::Deinitialize();
}

bool USyStateManagerSubsystem::RecordOperation(const FSyOperation& Operation)
{
    // 1. (可选) 基础验证
    if (!ValidateOperation(Operation))
    {
        UE_LOG(LogSyStateManager, Warning, TEXT("RecordOperation failed validation for OperationId: %s"), *Operation.OperationId.ToString());
        return false;
    }

    // 2. 创建记录
    FSyStateModificationRecord NewRecord(Operation);

    // 3. 添加到日志
    int32 NewIndex = ModificationLog.Num();
    ModificationLog.Add(NewRecord);
    
    // 4. 更新索引
    // 4.1 按目标类型索引
    if (Operation.Target.TargetTypeTag.IsValid())
    {
        TArray<int32>& Indices = TargetTypeIndex.FindOrAdd(Operation.Target.TargetTypeTag);
        Indices.Add(NewIndex);
    }
    
    // 4.2 按操作ID索引
    if (Operation.OperationId.IsValid())
    {
        OperationIdIndex.Add(Operation.OperationId, NewIndex);
    }
    
    // 5. 增量更新聚合快照（而非简单失效）
    if (Operation.Target.TargetTypeTag.IsValid())
    {
        FSyStateParameterSet& Snapshot = AggregatedCache.FindOrAdd(Operation.Target.TargetTypeTag);
        
        // 获取当前快照的 Map 形式
        TMap<FGameplayTag, TArray<FInstancedStruct>> SnapshotMap = Snapshot.GetMutableParametersMap();
        
        // 将新操作的修改增量合并到快照中
        for (const auto& ModPair : Operation.Modifier.StateModifications.GetParametersAsMap())
        {
            const FGameplayTag& StateTag = ModPair.Key;
            const TArray<FInstancedStruct>& NewParams = ModPair.Value;
            
            // 获取快照中对应Tag的现有参数数组
            TArray<FInstancedStruct>& SnapshotParams = SnapshotMap.FindOrAdd(StateTag);
            
            // 合并参数（与 AggregateRecordModifications 中的逻辑一致）
            for (const FInstancedStruct& SourceStruct : NewParams)
            {
                if (!SourceStruct.IsValid()) continue;
                
                const UScriptStruct* StructType = SourceStruct.GetScriptStruct();
                if (!StructType) continue;
                
                FInstancedStruct* ExistingPtr = SnapshotParams.FindByPredicate(
                    [&StructType](const FInstancedStruct& Existing)
                    {
                        return Existing.IsValid() && Existing.GetScriptStruct() == StructType;
                    });
                
                if (ExistingPtr)
                {
                    // 已存在相同类型的参数，合并或覆盖
                    const UScriptStruct* ListBaseType = FSyListParameterBase::StaticStruct();
                    if (StructType && StructType->IsChildOf(ListBaseType) && ExistingPtr->GetScriptStruct()->IsChildOf(ListBaseType))
                    {
                        // 列表类型 - 聚合
                        FSyListParameterBase* TargetList = ExistingPtr->GetMutablePtr<FSyListParameterBase>();
                        const FSyListParameterBase* SourceList = SourceStruct.GetPtr<FSyListParameterBase>();
                        if (TargetList && SourceList)
                        {
                            TargetList->AggregateItemsInternal(SourceList->GetListItemsInternal());
                        }
                    }
                    else
                    {
                        // 非列表类型 - 覆盖
                        *ExistingPtr = SourceStruct;
                    }
                }
                else
                {
                    // 不存在，添加新参数
                    SnapshotParams.Add(SourceStruct);
                }
            }
        }
        
        // 将更新后的 Map 同步回快照
        Snapshot.UpdateFromMap(SnapshotMap);
        
        // 更新版本号
        CacheVersions.Add(Operation.Target.TargetTypeTag, GlobalVersion);
        GlobalVersion++;
        
        UE_LOG(LogSyStateManager, VeryVerbose, TEXT("⚡ Incrementally updated snapshot for target tag: %s (Version: %d)"), 
            *Operation.Target.TargetTypeTag.ToString(), GlobalVersion - 1);
    }
    
    // 6. 广播事件
    // 6.1 精准广播给智能订阅者（推荐方式）
    BroadcastToSubscribers(NewRecord);
    
    // 6.2 全局广播（用于蓝图或需要监听所有变更的场景）
    if (OnStateModificationChanged.IsBound())
    {
        OnStateModificationChanged.Broadcast(NewRecord);
    }

    UE_LOG(LogSyStateManager, VeryVerbose, TEXT("✅ Operation recorded. RecordId: %s, OperationId: %s, Target: %s"), 
        *NewRecord.RecordId.ToString(), *Operation.OperationId.ToString(), *Operation.Target.TargetTypeTag.ToString());
    return true;
}

bool USyStateManagerSubsystem::UnloadOperation(const FGuid& OperationIdToUnload)
{
    if (!OperationIdToUnload.IsValid())
    {
        UE_LOG(LogSyStateManager, Warning, TEXT("UnloadOperation called with invalid GUID."));
        return false;
    }

    // 使用索引快速查找
    const int32* FoundIndexPtr = OperationIdIndex.Find(OperationIdToUnload);
    if (!FoundIndexPtr)
    {
        UE_LOG(LogSyStateManager, Log, TEXT("UnloadOperation: Operation with ID %s not found in log."), *OperationIdToUnload.ToString());
        return false;
    }

    int32 FoundIndex = *FoundIndexPtr;
    if (!ModificationLog.IsValidIndex(FoundIndex))
    {
        UE_LOG(LogSyStateManager, Error, TEXT("UnloadOperation: Invalid index %d for operation ID %s"), FoundIndex, *OperationIdToUnload.ToString());
        OperationIdIndex.Remove(OperationIdToUnload);
        return false;
    }

    // 保存副本用于广播
    FSyStateModificationRecord RecordCopy = ModificationLog[FoundIndex];
    FGameplayTag TargetTag = RecordCopy.Operation.Target.TargetTypeTag;
    
    // 从日志中移除（使用 RemoveAtSwap 提高效率）
    ModificationLog.RemoveAtSwap(FoundIndex);
    
    // 更新索引 - 由于使用了 RemoveAtSwap，需要更新被交换的元素的索引
    if (ModificationLog.IsValidIndex(FoundIndex))
    {
        // 被交换到当前位置的记录需要更新索引
        const FSyStateModificationRecord& SwappedRecord = ModificationLog[FoundIndex];
        
        // 更新操作ID索引
        if (SwappedRecord.Operation.OperationId.IsValid())
        {
            OperationIdIndex.Add(SwappedRecord.Operation.OperationId, FoundIndex);
        }
        
        // 更新目标类型索引
        if (SwappedRecord.Operation.Target.TargetTypeTag.IsValid())
        {
            TArray<int32>* IndicesPtr = TargetTypeIndex.Find(SwappedRecord.Operation.Target.TargetTypeTag);
            if (IndicesPtr)
            {
                int32 OldIndex = IndicesPtr->Find(ModificationLog.Num()); // 原来的最后一个索引
                if (OldIndex != INDEX_NONE)
                {
                    (*IndicesPtr)[OldIndex] = FoundIndex;
                }
            }
        }
    }
    
    // 从索引中移除被卸载的操作
    OperationIdIndex.Remove(OperationIdToUnload);
    
    if (TargetTag.IsValid())
    {
        TArray<int32>* IndicesPtr = TargetTypeIndex.Find(TargetTag);
        if (IndicesPtr)
        {
            IndicesPtr->Remove(FoundIndex);
        }
        
        // 重新计算该 TargetTag 的聚合快照
        RecalculateSnapshotForTarget(TargetTag);
        
        GlobalVersion++;
        
        UE_LOG(LogSyStateManager, VeryVerbose, TEXT("🔄 Recalculated snapshot for target tag: %s after unload"), 
            *TargetTag.ToString());
    }
    
    UE_LOG(LogSyStateManager, Log, TEXT("✅ Unloaded operation with ID: %s"), *OperationIdToUnload.ToString());
    
    // 广播变更
    if (OnStateModificationChanged.IsBound())
    {
        OnStateModificationChanged.Broadcast(RecordCopy);
    }
    
    return true;
}

// TODO: 替换为标准过滤规则，现在没用到所以懒得整
int32 USyStateManagerSubsystem::UnloadOperationsBySource(const FSyOperationSource& SourceToMatch)
{
    TArray<FSyStateModificationRecord> RecordsToBroadcast;
    TSet<FGameplayTag> AffectedTargetTags; // 收集受影响的目标类型
    int32 RemovedCount = 0;

    // Use RemoveAllSwap with predicate, collecting copies for broadcast
    RemovedCount = ModificationLog.RemoveAllSwap([
        &](const FSyStateModificationRecord& Record) -> bool 
        {
            if (bool bMatch = (Record.Operation.Source.SourceTypeTag == SourceToMatch.SourceTypeTag))
            {   
                RecordsToBroadcast.Add(Record); // Add copy before potential removal
                if (Record.Operation.Target.TargetTypeTag.IsValid())
                {
                    AffectedTargetTags.Add(Record.Operation.Target.TargetTypeTag); // 记录受影响的目标
                }
                return true; // Mark for removal
            }
            return false; 
        });

    if (RemovedCount > 0)
    {
        UE_LOG(LogSyStateManager, Log, TEXT("Unloaded %d operations matching source (Tag: %s)."), 
            RemovedCount, 
            *SourceToMatch.SourceTypeTag.ToString());

        // 重建索引并重新计算受影响目标的快照
        OperationIdIndex.Empty();
        TargetTypeIndex.Empty();
        for (int32 i = 0; i < ModificationLog.Num(); ++i)
        {
            const FSyStateModificationRecord& Record = ModificationLog[i];
            if (Record.Operation.OperationId.IsValid())
            {
                OperationIdIndex.Add(Record.Operation.OperationId, i);
            }
            if (Record.Operation.Target.TargetTypeTag.IsValid())
            {
                TargetTypeIndex.FindOrAdd(Record.Operation.Target.TargetTypeTag).Add(i);
            }
        }
        
        // 重新计算所有受影响目标的聚合快照
        for (const FGameplayTag& AffectedTag : AffectedTargetTags)
        {
            RecalculateSnapshotForTarget(AffectedTag);
        }
        
        GlobalVersion++;

        // Broadcast the change for each removed record using the unified delegate
        if (OnStateModificationChanged.IsBound())
        {
            for (const FSyStateModificationRecord& RemovedRecord : RecordsToBroadcast)
            {
                 OnStateModificationChanged.Broadcast(RemovedRecord);
            }
        }
    }
    else
    {
        UE_LOG(LogSyStateManager, Log, TEXT("UnloadOperationsBySource: No operations found matching source (Tag: %s)."), 
            *SourceToMatch.SourceTypeTag.ToString());
    }

    return RemovedCount;
}

FSyStateParameterSet USyStateManagerSubsystem::GetAggregatedModifications(const FGameplayTag& TargetFilterTag /* TODO: 添加 SourceFilterTag */) const
{
    // ===== 直接返回预聚合快照 =====
    if (TargetFilterTag.IsValid())
    {
        // 查找预聚合的快照
        const FSyStateParameterSet* Snapshot = AggregatedCache.Find(TargetFilterTag);
        if (Snapshot)
        {
            UE_LOG(LogSyStateManager, VeryVerbose, TEXT("⚡ Returning pre-aggregated snapshot for target tag: %s"), 
                *TargetFilterTag.ToString());
            return *Snapshot;
        }
        
        // 快照不存在，说明还没有该目标类型的操作记录
        UE_LOG(LogSyStateManager, VeryVerbose, TEXT("No snapshot found for target tag: %s, returning empty set"), 
            *TargetFilterTag.ToString());
        return FSyStateParameterSet();
    }

    // 没有目标过滤时，手动聚合所有记录（保持向后兼容）
    UE_LOG(LogSyStateManager, VeryVerbose, TEXT("No target filter provided, manually aggregating all records..."));
    
    FSyStateParameterSet AggregatedResult;
    TMap<FGameplayTag, TArray<FInstancedStruct>> AggregatedParamsMap;
    
    for (const FSyStateModificationRecord& Record : ModificationLog)
    {
        AggregateRecordModifications(Record, AggregatedParamsMap);
    }
    
    AggregatedResult = AggregatedParamsMap;
    return AggregatedResult;
}

// 辅助方法：聚合单个记录的修改
void USyStateManagerSubsystem::AggregateRecordModifications(
    const FSyStateModificationRecord& Record,
    TMap<FGameplayTag, TArray<FInstancedStruct>>& OutAggregatedMap) const
{
    for (const auto& Pair : Record.Operation.Modifier.StateModifications.GetParametersAsMap())
    {
        const FGameplayTag& StateTag = Pair.Key;
        const TArray<FInstancedStruct>& ParamsToMerge = Pair.Value;

        TArray<FInstancedStruct>& ExistingParams = OutAggregatedMap.FindOrAdd(StateTag);

        for (const FInstancedStruct& SourceStruct : ParamsToMerge)
        {
            if (!SourceStruct.IsValid()) continue;

            const UScriptStruct* StructType = SourceStruct.GetScriptStruct();
            if (!StructType) continue;

            FInstancedStruct* TargetStructPtr = ExistingParams.FindByPredicate(
                [&StructType](const FInstancedStruct& ExistingStruct)
                {
                    return ExistingStruct.IsValid() && ExistingStruct.GetScriptStruct() == StructType;
                });

            if (TargetStructPtr)
            {
                const UScriptStruct* ListBaseType = FSyListParameterBase::StaticStruct();

                if (StructType && StructType->IsChildOf(ListBaseType) && TargetStructPtr->GetScriptStruct()->IsChildOf(ListBaseType))
                {
                    FSyListParameterBase* TargetListBase = TargetStructPtr->GetMutablePtr<FSyListParameterBase>();
                    const FSyListParameterBase* SourceListBase = SourceStruct.GetPtr<FSyListParameterBase>();

                    if (TargetListBase && SourceListBase)
                    {
                        const TArray<FInstancedStruct>& ItemsToAggregate = SourceListBase->GetListItemsInternal();
                        TargetListBase->AggregateItemsInternal(ItemsToAggregate);
                    }
                    else
                    {
                        UE_LOG(LogSyStateManager, Warning, TEXT("Failed to get FSyListParameterBase pointers for aggregation for type: %s. Falling back to overwrite."), *StructType->GetName());
                        *TargetStructPtr = SourceStruct;
                    }
                }
                else
                {
                    *TargetStructPtr = SourceStruct;
                }
            }
            else
            {
                ExistingParams.Add(SourceStruct);
            }
        }
    }
}

void USyStateManagerSubsystem::RecalculateSnapshotForTarget(const FGameplayTag& TargetTag)
{
    if (!TargetTag.IsValid())
    {
        return;
    }

    // 清空现有快照
    FSyStateParameterSet& Snapshot = AggregatedCache.FindOrAdd(TargetTag);
    Snapshot.ClearAllStateParams();

    // 使用索引获取该目标类型的所有记录
    const TArray<int32>* IndicesPtr = TargetTypeIndex.Find(TargetTag);
    if (!IndicesPtr || IndicesPtr->Num() == 0)
    {
        // 没有相关记录，快照保持为空
        CacheVersions.Add(TargetTag, GlobalVersion);
        UE_LOG(LogSyStateManager, Verbose, TEXT("Recalculated empty snapshot for target tag: %s"), *TargetTag.ToString());
        return;
    }

    // 重新聚合所有相关记录
    TMap<FGameplayTag, TArray<FInstancedStruct>> AggregatedMap;
    for (int32 Index : *IndicesPtr)
    {
        if (ModificationLog.IsValidIndex(Index))
        {
            AggregateRecordModifications(ModificationLog[Index], AggregatedMap);
        }
    }

    // 更新快照
    Snapshot = AggregatedMap;
    CacheVersions.Add(TargetTag, GlobalVersion);

    UE_LOG(LogSyStateManager, Verbose, TEXT("✅ Recalculated snapshot for target tag: %s with %d records"), 
        *TargetTag.ToString(), IndicesPtr->Num());
}

const TArray<FSyStateModificationRecord>& USyStateManagerSubsystem::GetAllModifications_Simple() const
{
    return ModificationLog;
}


bool USyStateManagerSubsystem::SaveLog()
{
    // 创建或获取 SaveGame 对象
    USyStateManagerSaveGame* SaveGameObject = nullptr;
    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex))
    {
        SaveGameObject = Cast<USyStateManagerSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex));
    }
    
    // 如果不存在或加载失败，创建一个新的
    if (!SaveGameObject)
    {
        SaveGameObject = Cast<USyStateManagerSaveGame>(UGameplayStatics::CreateSaveGameObject(USyStateManagerSaveGame::StaticClass()));
        if (!SaveGameObject)
        {
            UE_LOG(LogSyStateManager, Error, TEXT("Failed to create SaveGameObject!"));
            return false;
        }
    }

    // 将当前的日志数据复制到 SaveGame 对象中
    SaveGameObject->SavedModificationLog = ModificationLog;
    SaveGameObject->SaveGameVersion = TEXT("1.0"); // 更新版本号（如果需要）

    // 保存到磁盘
    bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGameObject, SaveSlotName, UserIndex);

    if (bSuccess)
    {
        UE_LOG(LogSyStateManager, Log, TEXT("State Manager Log saved successfully to slot: %s"), *SaveSlotName);
    }
    else
    {
        UE_LOG(LogSyStateManager, Error, TEXT("Failed to save State Manager Log to slot: %s"), *SaveSlotName);
    }

    return bSuccess;
}

bool USyStateManagerSubsystem::LoadLog()
{
    // 检查存档是否存在
    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex))
    {
        // 加载存档对象
        USyStateManagerSaveGame* LoadedSaveGame = Cast<USyStateManagerSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex));

        if (LoadedSaveGame)
        {
            // 从存档对象恢复日志数据
            // 这里直接覆盖当前的 ModificationLog。如果需要合并或更复杂的逻辑，在此处修改。
            ModificationLog = LoadedSaveGame->SavedModificationLog;
            UE_LOG(LogSyStateManager, Log, TEXT("State Manager Log loaded successfully from slot: %s. %d records loaded."), 
                *SaveSlotName, ModificationLog.Num());
            
            // TODO: 加载后可能需要重新广播事件或通知相关系统状态已恢复？
            // 这取决于下游系统的设计。
            
            return true;
        }
        else
        {
            UE_LOG(LogSyStateManager, Error, TEXT("Failed to load State Manager Log from slot: %s (Cast Failed)"), *SaveSlotName);
        }
    }
    else
    {
        UE_LOG(LogSyStateManager, Log, TEXT("No existing save game found for State Manager Log in slot: %s. Starting with an empty log."), *SaveSlotName);
    }

    // 如果加载失败或存档不存在，确保日志是空的
    ModificationLog.Empty();
    return false;
}


void USyStateManagerSubsystem::AddRecordAndBroadcast(const FSyStateModificationRecord& Record)
{
    ModificationLog.Add(Record);
    // Broadcast using the unified delegate
    if (OnStateModificationChanged.IsBound())
    {
        OnStateModificationChanged.Broadcast(Record);
    }
}

bool USyStateManagerSubsystem::ValidateOperation(const FSyOperation& Operation) const
{
    if (!Operation.OperationId.IsValid())
    {
        UE_LOG(LogSyStateManager, Warning, TEXT("ValidateOperation failed: OperationId is invalid."));
        return false;
    }
    if (!Operation.Target.TargetTypeTag.IsValid())
    {        
        UE_LOG(LogSyStateManager, Warning, TEXT("ValidateOperation failed: TargetTypeTag is invalid for OpId: %s."), *Operation.OperationId.ToString());
        return false;
    }
    // Add more validation as needed (e.g., check source, modifier)
    return true;
}

// ===== 智能订阅实现 =====

void USyStateManagerSubsystem::SubscribeToTargetType(
    FGameplayTag TargetTypeTag, 
    UObject* Subscriber,
    FOnStateModificationChangedNative Delegate)
{
    if (!TargetTypeTag.IsValid())
    {
        UE_LOG(LogSyStateManager, Warning, TEXT("SubscribeToTargetType: Invalid TargetTypeTag"));
        return;
    }
    
    if (!Subscriber)
    {
        UE_LOG(LogSyStateManager, Warning, TEXT("SubscribeToTargetType: Null Subscriber"));
        return;
    }
    
    if (!Delegate.IsBound())
    {
        UE_LOG(LogSyStateManager, Warning, TEXT("SubscribeToTargetType: Delegate not bound"));
        return;
    }
    
    TArray<FSubscriberInfo>& Subscribers = TargetTypeSubscribers.FindOrAdd(TargetTypeTag);
    
    // 检查是否已经订阅
    for (const FSubscriberInfo& Info : Subscribers)
    {
        if (Info.Subscriber == Subscriber)
        {
            UE_LOG(LogSyStateManager, Verbose, TEXT("Subscriber %s already subscribed to target type: %s"), 
                *Subscriber->GetName(), *TargetTypeTag.ToString());
            return;
        }
    }
    
    Subscribers.Add(FSubscriberInfo(Subscriber, Delegate));
    
    UE_LOG(LogSyStateManager, Log, TEXT("✅ Subscriber %s subscribed to target type: %s"), 
        *Subscriber->GetName(), *TargetTypeTag.ToString());
}

void USyStateManagerSubsystem::UnsubscribeFromTargetType(FGameplayTag TargetTypeTag, UObject* Subscriber)
{
    if (!TargetTypeTag.IsValid() || !Subscriber)
    {
        return;
    }
    
    TArray<FSubscriberInfo>* SubscribersPtr = TargetTypeSubscribers.Find(TargetTypeTag);
    if (!SubscribersPtr)
    {
        return;
    }
    
    int32 RemovedCount = SubscribersPtr->RemoveAll([Subscriber](const FSubscriberInfo& Info)
    {
        return Info.Subscriber == Subscriber;
    });
    
    if (RemovedCount > 0)
    {
        UE_LOG(LogSyStateManager, Log, TEXT("Unsubscribed %s from target type: %s"), 
            *Subscriber->GetName(), *TargetTypeTag.ToString());
        
        // 如果该目标类型没有订阅者了，移除整个条目
        if (SubscribersPtr->Num() == 0)
        {
            TargetTypeSubscribers.Remove(TargetTypeTag);
        }
    }
}

void USyStateManagerSubsystem::UnsubscribeAll(UObject* Subscriber)
{
    if (!Subscriber)
    {
        return;
    }
    
    int32 TotalRemovedCount = 0;
    TArray<FGameplayTag> EmptyTags;
    
    for (auto& Pair : TargetTypeSubscribers)
    {
        int32 RemovedCount = Pair.Value.RemoveAll([Subscriber](const FSubscriberInfo& Info)
        {
            return Info.Subscriber == Subscriber;
        });
        
        TotalRemovedCount += RemovedCount;
        
        if (Pair.Value.Num() == 0)
        {
            EmptyTags.Add(Pair.Key);
        }
    }
    
    // 移除空的订阅列表
    for (const FGameplayTag& Tag : EmptyTags)
    {
        TargetTypeSubscribers.Remove(Tag);
    }
    
    if (TotalRemovedCount > 0)
    {
        UE_LOG(LogSyStateManager, Log, TEXT("Unsubscribed %s from all target types (removed %d subscriptions)"), 
            *Subscriber->GetName(), TotalRemovedCount);
    }
}

void USyStateManagerSubsystem::BroadcastToSubscribers(const FSyStateModificationRecord& Record)
{
    const FGameplayTag& TargetTag = Record.Operation.Target.TargetTypeTag;
    if (!TargetTag.IsValid())
    {
        return;
    }
    
    TArray<FSubscriberInfo>* SubscribersPtr = TargetTypeSubscribers.Find(TargetTag);
    if (!SubscribersPtr || SubscribersPtr->Num() == 0)
    {
        return;
    }
    
    // 清理无效订阅者
    int32 InvalidCount = SubscribersPtr->RemoveAll([](const FSubscriberInfo& Info)
    {
        return !Info.IsValid();
    });
    
    if (InvalidCount > 0)
    {
        UE_LOG(LogSyStateManager, Verbose, TEXT("Cleaned up %d invalid subscribers for target type: %s"), 
            InvalidCount, *TargetTag.ToString());
    }
    
    // 广播给有效的订阅者
    int32 BroadcastCount = 0;
    for (const FSubscriberInfo& Info : *SubscribersPtr)
    {
        if (Info.IsValid() && Info.Delegate.IsBound())
        {
            Info.Delegate.Execute(Record);
            BroadcastCount++;
        }
    }
    
    UE_LOG(LogSyStateManager, VeryVerbose, TEXT("📢 Broadcasted to %d subscribers for target type: %s"), 
        BroadcastCount, *TargetTag.ToString());
}

void USyStateManagerSubsystem::CleanupInvalidSubscribers()
{
    int32 TotalCleaned = 0;
    TArray<FGameplayTag> EmptyTags;
    
    for (auto& Pair : TargetTypeSubscribers)
    {
        int32 CleanedCount = Pair.Value.RemoveAll([](const FSubscriberInfo& Info)
        {
            return !Info.IsValid();
        });
        
        TotalCleaned += CleanedCount;
        
        if (Pair.Value.Num() == 0)
        {
            EmptyTags.Add(Pair.Key);
        }
    }
    
    for (const FGameplayTag& Tag : EmptyTags)
    {
        TargetTypeSubscribers.Remove(Tag);
    }
    
    if (TotalCleaned > 0)
    {
        UE_LOG(LogSyStateManager, Log, TEXT("🧹 Cleaned up %d invalid subscribers"), TotalCleaned);
    }
}
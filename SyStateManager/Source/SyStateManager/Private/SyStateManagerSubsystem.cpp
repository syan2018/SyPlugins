// Copyright Epic Games, Inc. All Rights Reserved.

#include "SyStateManagerSubsystem.h"
#include "OperationTypes.h" // 需要 FSyOperation 定义
#include "StateParameterTypes.h" // 需要 FSyStateParameterSet 定义
#include "GameplayTagContainer.h" // 需要 FGameplayTag 定义
#include "Logging/LogMacros.h" // 用于日志输出
#include "Runtime/Launch/Resources/Version.h" // 用于 ENGINE_MAJOR_VERSION 等宏
#include "SyStateManagerSaveGame.h" // 包含自定义 SaveGame 类
#include "Kismet/GameplayStatics.h" // 包含 GameplayStatics
#include "StructUtils/InstancedStruct.h"
#include "Metadatas/ListMetadataValueTypes.h" // *** 包含新的列表基类头文件 ***
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
    
    // 5. 使相关缓存失效
    if (Operation.Target.TargetTypeTag.IsValid())
    {
        CacheVersions.Remove(Operation.Target.TargetTypeTag);
        GlobalVersion++;
    }
    
    // 6. 广播事件
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
        
        // 使缓存失效
        CacheVersions.Remove(TargetTag);
        GlobalVersion++;
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
    int32 RemovedCount = 0;

    // Use RemoveAllSwap with predicate, collecting copies for broadcast
    RemovedCount = ModificationLog.RemoveAllSwap([
        &](const FSyStateModificationRecord& Record) -> bool 
        {
            if (bool bMatch = (Record.Operation.Source.SourceTypeTag == SourceToMatch.SourceTypeTag))
            {   
                RecordsToBroadcast.Add(Record); // Add copy before potential removal
                return true; // Mark for removal
            }
            return false; 
        });

    if (RemovedCount > 0)
    {
        UE_LOG(LogSyStateManager, Log, TEXT("Unloaded %d operations matching source (Tag: %s)."), 
            RemovedCount, 
            *SourceToMatch.SourceTypeTag.ToString());

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
    // ===== 缓存检查 =====
    if (TargetFilterTag.IsValid())
    {
        // 检查缓存是否有效
        const FSyStateParameterSet* CachedResult = AggregatedCache.Find(TargetFilterTag);
        const int32* CachedVersion = CacheVersions.Find(TargetFilterTag);
        
        if (CachedResult && CachedVersion && *CachedVersion == GlobalVersion)
        {
            UE_LOG(LogSyStateManager, VeryVerbose, TEXT("⚡ Cache hit for target tag: %s"), *TargetFilterTag.ToString());
            return *CachedResult;
        }
    }

    UE_LOG(LogSyStateManager, VeryVerbose, TEXT("Cache miss for target tag: %s, computing aggregation..."), 
        TargetFilterTag.IsValid() ? *TargetFilterTag.ToString() : TEXT("Invalid"));

    FSyStateParameterSet AggregatedResult;
    TMap<FGameplayTag, TArray<FInstancedStruct>> AggregatedParamsMap;

    // ===== 使用索引优化查询 =====
    // 如果提供了目标类型过滤，使用索引只遍历相关记录
    if (TargetFilterTag.IsValid())
    {
        const TArray<int32>* IndicesPtr = TargetTypeIndex.Find(TargetFilterTag);
        if (IndicesPtr)
        {
            for (int32 Index : *IndicesPtr)
            {
                if (!ModificationLog.IsValidIndex(Index))
                {
                    UE_LOG(LogSyStateManager, Warning, TEXT("Invalid index %d found in TargetTypeIndex for tag %s"), 
                        Index, *TargetFilterTag.ToString());
                    continue;
                }
                
                const FSyStateModificationRecord& Record = ModificationLog[Index];
                AggregateRecordModifications(Record, AggregatedParamsMap);
            }
        }
    }
    else
    {
        // 没有过滤条件时，遍历所有记录（保持向后兼容）
        for (const FSyStateModificationRecord& Record : ModificationLog)
        {
            AggregateRecordModifications(Record, AggregatedParamsMap);
        }
    }
    
    // 将聚合后的 Map 赋值给结果结构体
    AggregatedResult = AggregatedParamsMap;

    // ===== 更新缓存 =====
    if (TargetFilterTag.IsValid())
    {
        // 使用 const_cast 绕过 const 限制（这是合理的，因为缓存不影响逻辑结果）
        USyStateManagerSubsystem* MutableThis = const_cast<USyStateManagerSubsystem*>(this);
        MutableThis->AggregatedCache.Add(TargetFilterTag, AggregatedResult);
        MutableThis->CacheVersions.Add(TargetFilterTag, GlobalVersion);
        
        UE_LOG(LogSyStateManager, Verbose, TEXT("✅ Cached aggregation result for target tag: %s (Version: %d)"), 
            *TargetFilterTag.ToString(), GlobalVersion);
    }

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
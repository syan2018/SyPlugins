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

    // 3. 添加到日志并广播
    AddRecordAndBroadcast(NewRecord);

    // UE_LOG(LogSyStateManager, Verbose, TEXT("Operation recorded. RecordId: %s, OperationId: %s"), *NewRecord.RecordId.ToString(), *Operation.OperationId.ToString());
    return true;
}

bool USyStateManagerSubsystem::UnloadOperation(const FGuid& OperationIdToUnload)
{
    if (!OperationIdToUnload.IsValid())
    {
        UE_LOG(LogSyStateManager, Warning, TEXT("UnloadOperation called with invalid GUID."));
        return false;
    }

    int32 FoundIndex = ModificationLog.IndexOfByPredicate(
        [&](const FSyStateModificationRecord& Record){ return Record.Operation.OperationId == OperationIdToUnload; });

    if (FoundIndex != INDEX_NONE)
    {
        FSyStateModificationRecord RecordCopy = ModificationLog[FoundIndex]; // Make a copy before removal
        ModificationLog.RemoveAtSwap(FoundIndex); // Use RemoveAtSwap for efficiency
        
        UE_LOG(LogSyStateManager, Log, TEXT("Unloaded operation with ID: %s"), *OperationIdToUnload.ToString());
        
        // Broadcast the change using the unified delegate
        if (OnStateModificationChanged.IsBound())
        {
            OnStateModificationChanged.Broadcast(RecordCopy); // Broadcast the removed record
        }
        return true;
    }
    else
    {
        UE_LOG(LogSyStateManager, Log, TEXT("UnloadOperation: Operation with ID %s not found in log."), *OperationIdToUnload.ToString());
    }

    return false;
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
    FSyStateParameterSet AggregatedResult;
    // 使用一个临时的Map来聚合参数，处理覆盖逻辑
    TMap<FGameplayTag, TArray<FInstancedStruct>> AggregatedParamsMap;

    // TODO: [拓展] 性能考量: 如果 ModificationLog 非常大，此线性扫描可能成为瓶颈。
    for (const FSyStateModificationRecord& Record : ModificationLog)
    {
        // --- 筛选阶段 ---
        // 1. TODO: [拓展] 源筛选

        // 2. 目标类型筛选
        if (TargetFilterTag.IsValid() && Record.Operation.Target.TargetTypeTag != TargetFilterTag)
        {
            continue;
        }

        // 3. TODO: [拓展] 更复杂的目标参数匹配

        // --- 聚合阶段 ---
        // 将当前记录的 Modifier 参数合并到临时 Map 中
        // 后出现的记录会覆盖或合并先出现的记录中相同 StateTag 的条目
        // TODO: 处理冲突逻辑，相同来源覆盖、不同来源互斥
        // 不过在这里处理，异步会不会爆炸啊，最好在Record时就？最好套个TryGet方法检查冲突
        for (const auto& Pair : Record.Operation.Modifier.StateModifications.GetParametersAsMap())
        {
            const FGameplayTag& StateTag = Pair.Key;
            const TArray<FInstancedStruct>& ParamsToMerge = Pair.Value;

            // 查找临时 Map 中是否已有该 StateTag 的条目
            TArray<FInstancedStruct>& ExistingParams = AggregatedParamsMap.FindOrAdd(StateTag);

            // 遍历当前记录中该 StateTag 下的所有待合并参数
            for (const FInstancedStruct& SourceStruct : ParamsToMerge)
            {
                if (!SourceStruct.IsValid()) continue; // 跳过无效的源结构体

                const UScriptStruct* StructType = SourceStruct.GetScriptStruct();
                if (!StructType) continue; // 跳过没有 ScriptStruct 的情况

                // 尝试在 ExistingParams 中查找具有相同类型的结构体
                FInstancedStruct* TargetStructPtr = ExistingParams.FindByPredicate(
                    [&StructType](const FInstancedStruct& ExistingStruct)
                    {
                        return ExistingStruct.IsValid() && ExistingStruct.GetScriptStruct() == StructType;
                    });

                // 如果找到了相同类型的参数
                if (TargetStructPtr)
                {
                    const UScriptStruct* ListBaseType = FSyListParameterBase::StaticStruct(); // 获取列表基类类型

                    // --- 检查是否为列表类型并应用列表聚合逻辑 ---
                    // （这里的 StructType 是 SourceStruct.GetScriptStruct()）
                    if (StructType && StructType->IsChildOf(ListBaseType) && TargetStructPtr->GetScriptStruct()->IsChildOf(ListBaseType)) // 确保目标和源都是列表类型
                    {
                        UE_LOG(LogSyStateManager, Verbose, TEXT("Aggregating list type %s using virtual aggregation logic."), *StructType->GetName());

                        // 获取目标和源的可变/常量基类指针
                        FSyListParameterBase* TargetListBase = TargetStructPtr->GetMutablePtr<FSyListParameterBase>();
                        const FSyListParameterBase* SourceListBase = SourceStruct.GetPtr<FSyListParameterBase>();

                        if (TargetListBase && SourceListBase)
                        {
                            // 通过虚函数获取源列表项
                            const TArray<FInstancedStruct>& ItemsToAggregate = SourceListBase->GetListItemsInternal();
                            // 通过虚函数执行聚合
                            TargetListBase->AggregateItemsInternal(ItemsToAggregate);
                        }
                        else
                        {
                            // 如果类型转换失败，记录警告并回退到覆盖
                            UE_LOG(LogSyStateManager, Warning, TEXT("Failed to get FSyListParameterBase pointers for aggregation for type: %s. Falling back to overwrite."), *StructType->GetName());
                            *TargetStructPtr = SourceStruct;
                        }
                    }
                    // --- （可选）处理其他需要特殊聚合的非列表类型 ---
                    // --- 默认行为：对于非列表且无特殊规则的类型，直接覆盖 ---
                    else
                    {
                        UE_LOG(LogSyStateManager, Verbose, TEXT("Aggregating non-list type %s using default behavior (overwrite)."), *StructType->GetName());
                        *TargetStructPtr = SourceStruct;
                    }
                }
                // 如果没有找到相同类型的参数
                else
                {
                    // 直接将新的参数添加到 ExistingParams 数组中
                    ExistingParams.Add(SourceStruct);
                }
            }
        }
    }
    
    // 将聚合后的 Map 赋值给结果结构体
    AggregatedResult = AggregatedParamsMap;

    return AggregatedResult;
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
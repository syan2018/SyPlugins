// Copyright Epic Games, Inc. All Rights Reserved.

#include "SyStateManagerSubsystem.h"
#include "OperationTypes.h" // 需要 FSyOperation 定义
#include "StateParameterTypes.h" // 需要 FSyStateParameterSet 定义
#include "GameplayTagContainer.h" // 需要 FGameplayTag 定义
#include "Logging/LogMacros.h" // 用于日志输出
#include "Runtime/Launch/Resources/Version.h" // 用于 ENGINE_MAJOR_VERSION 等宏
#include "SyStateManagerSaveGame.h" // 包含自定义 SaveGame 类
#include "Kismet/GameplayStatics.h" // 包含 GameplayStatics

// 定义一个简单的日志分类
DEFINE_LOG_CATEGORY_STATIC(LogSyStateManager, Log, All); // 启用日志以方便调试

void USyStateManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogSyStateManager, Log, TEXT("SyStateManagerSubsystem Initialized."));
    // 在子系统初始化时尝试加载存档
    LoadLog();
}

void USyStateManagerSubsystem::Deinitialize()
{
    UE_LOG(LogSyStateManager, Log, TEXT("SyStateManagerSubsystem Deinitializing. Saving log..."));
    // 在子系统反初始化前尝试保存日志（确保游戏退出时也能保存）
    SaveLog();
    ModificationLog.Empty();
    OnStateModificationRecorded.Clear();
    Super::Deinitialize();
}

bool USyStateManagerSubsystem::RecordOperation(const FSyOperation& Operation)
{
    // 1. (可选) 基础验证
    if (!ValidateOperation(Operation))
    {
        // UE_LOG(LogSyStateManager, Warning, TEXT("RecordOperation failed validation for OperationId: %s"), *Operation.OperationId.ToString());
        return false;
    }

    // 2. 创建记录
    FSyStateModificationRecord NewRecord(Operation);

    // 3. 添加到日志并广播
    AddRecordAndBroadcast(NewRecord);

    // UE_LOG(LogSyStateManager, Verbose, TEXT("Operation recorded. RecordId: %s, OperationId: %s"), *NewRecord.RecordId.ToString(), *Operation.OperationId.ToString());
    return true;
}

FSyStateParameterSet USyStateManagerSubsystem::GetAggregatedModifications(const FGameplayTag& TargetFilterTag /* TODO: 添加 SourceFilterTag */) const
{
    FSyStateParameterSet AggregatedParams;

    // TODO: [拓展] 性能考量: 如果 ModificationLog 非常大，此线性扫描可能成为瓶颈。
    for (const FSyStateModificationRecord& Record : ModificationLog)
    {
        // --- 筛选阶段 ---

        // 1. TODO: [拓展] 源筛选 (Source Filtering)
        // if (!SourceMatches(Record.Operation.Source, SourceFilterTag))
        // {
        //     continue;
        // }

        // 2. 目标类型筛选 (Target Type Filtering)
        // 如果 TargetFilterTag 有效，则要求操作的目标类型标签必须精确匹配
        if (TargetFilterTag.IsValid() && Record.Operation.Target.TargetTypeTag != TargetFilterTag)
        {
             continue; // 不匹配，跳过此记录
        }
        // 如果 TargetFilterTag 无效 (例如 EmptyTag)，则认为匹配所有目标类型 (取决于具体需求，当前为不筛选)
        // else if (!TargetFilterTag.IsValid()) { /* 匹配所有 */ }


        // 3. TODO: [拓展] 更复杂的目标参数匹配 (Target Parameter Matching)
        // 可能需要传入目标实体的上下文信息进行比较
        // if (!TargetParametersMatch(Record.Operation.Target.Parameters, TargetContext))
        // {
        //     continue;
        // }

        // --- 聚合阶段 ---
        // 如果记录通过了所有筛选条件，则将其 Modifier 中的 StateModifications 合并到结果中

        // 检查 Operation.Modifier.StateModifications 是否有效以及内部 Map 是否存在
        // (假设 FSyStateParameterSet 提供了类似 IsValid() 或 GetStateParamsMap() 返回 const 引用/指针的方法)
        const auto& StateParamsMap = Record.Operation.Modifier.StateModifications.GetStateParamsMap(); // 假设有这个方法

        // 遍历当前记录 Modifier 中的所有状态修改
        for (const auto& Pair : StateParamsMap)
        {
            const FGameplayTag& StateTag = Pair.Key;
            const TArray<FSyInstancedStruct>& ParamsToAdd = Pair.Value.Params;
            // 将这些参数添加到聚合结果的对应 StateTag 下
            AggregatedParams.AddStateParams(StateTag, ParamsToAdd);
        }
        // 或者如果 FSyStateParameterSet 提供了 Append 方法：
        // AggregatedParams.Append(Record.Operation.Modifier.StateModifications);
    }

    return AggregatedParams;
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
    if (OnStateModificationRecorded.IsBound())
    {
        OnStateModificationRecorded.Broadcast(Record);
    }
}

bool USyStateManagerSubsystem::ValidateOperation(const FSyOperation& Operation) const
{
    if (!Operation.OperationId.IsValid())
    {
        return false;
    }
    return true;
}
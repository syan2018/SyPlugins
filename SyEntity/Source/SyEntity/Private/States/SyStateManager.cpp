#include "States/SyStateManager.h"

void USyStateManager::UpdateEntityState(const FGuid& EntityId, const FGameplayTag& StateTag, bool bNewValue)
{
    if (!EntityId.IsValid())
    {
        return;
    }
    
    // 获取或创建实体的状态数据
    FSyEntityStateData& EntityData = GlobalEntityStates.FindOrAdd(EntityId);
    
    // 检查状态是否已存在且值相同
    bool* ExistingValue = EntityData.States.Find(StateTag);
    if (ExistingValue && *ExistingValue == bNewValue)
    {
        return; // 状态未变化，无需更新
    }
    
    // 更新状态
    EntityData.States.Add(StateTag, bNewValue);
    
    // 广播状态变更事件
    OnGlobalStateChanged.Broadcast(EntityId, StateTag, bNewValue);
}

bool USyStateManager::GetEntityState(const FGuid& EntityId, const FGameplayTag& StateTag, bool& bFound) const
{
    bFound = false;
    
    if (!EntityId.IsValid())
    {
        return false;
    }
    
    // 查找实体的状态数据
    const FSyEntityStateData* EntityData = GlobalEntityStates.Find(EntityId);
    if (!EntityData)
    {
        return false;
    }
    
    // 查找特定状态
    const bool* Value = EntityData->States.Find(StateTag);
    if (!Value)
    {
        return false;
    }
    
    bFound = true;
    return *Value;
}

TMap<FGameplayTag, bool> USyStateManager::GetAllEntityStates(const FGuid& EntityId, bool& bFound) const
{
    bFound = false;
    TMap<FGameplayTag, bool> Result;
    
    if (!EntityId.IsValid())
    {
        return Result;
    }
    
    // 查找实体的状态数据
    const FSyEntityStateData* EntityData = GlobalEntityStates.Find(EntityId);
    if (!EntityData)
    {
        return Result;
    }
    
    bFound = true;
    return EntityData->States;
}

void USyStateManager::RegisterEntity(const FGuid& EntityId)
{
    if (!EntityId.IsValid())
    {
        return;
    }
    
    // 如果实体不存在，则创建一个空的状态数据
    if (!GlobalEntityStates.Contains(EntityId))
    {
        FSyEntityStateData NewData;
        GlobalEntityStates.Add(EntityId, NewData);
    }
}

void USyStateManager::UnregisterEntity(const FGuid& EntityId)
{
    if (!EntityId.IsValid())
    {
        return;
    }
    
    // 移除实体的所有状态
    GlobalEntityStates.Remove(EntityId);
} 
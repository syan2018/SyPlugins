#include "Registry/SyEntityRegistry.h"
#include "SyEntityComponent.h"
#include "SyCore/Public/Identity/SyIdentityComponent.h"

void USyEntityRegistry::RegisterEntity(USyEntityComponent* EntityComponent)
{
    if (!EntityComponent)
    {
        return;
    }
    
    // 获取实体ID
    FGuid EntityId = EntityComponent->GetEntityId();
    if (!EntityId.IsValid())
    {
        return;
    }
    
    // 添加到ID索引
    EntityIdMap.Add(EntityId, EntityComponent);
    
    // 更新Tag索引
    AddEntityToTagMap(EntityComponent);
}

void USyEntityRegistry::UnregisterEntity(USyEntityComponent* EntityComponent)
{
    if (!EntityComponent)
    {
        return;
    }
    
    // 从ID索引中移除
    FGuid EntityId = EntityComponent->GetEntityId();
    if (EntityId.IsValid())
    {
        EntityIdMap.Remove(EntityId);
    }
    
    // 从Tag索引中移除
    RemoveEntityFromTagMap(EntityComponent);
}

USyEntityComponent* USyEntityRegistry::GetEntityById(const FGuid& EntityId) const
{
    if (!EntityId.IsValid())
    {
        return nullptr;
    }
    
    const TObjectPtr<USyEntityComponent>* EntityPtr = EntityIdMap.Find(EntityId);
    if (EntityPtr)
    {
        return *EntityPtr;
    }
    
    return nullptr;
}

TArray<USyEntityComponent*> USyEntityRegistry::GetEntitiesByTag(const FGameplayTag& EntityTag) const
{
    TArray<USyEntityComponent*> Result;
    
    // 遍历所有实体，检查是否包含指定Tag
    for (const auto& Pair : EntityIdMap)
    {
        USyEntityComponent* Entity = Pair.Value;
        if (Entity && Entity->GetEntityTags().HasTag(EntityTag))
        {
            Result.Add(Entity);
        }
    }
    
    return Result;
}

TArray<USyEntityComponent*> USyEntityRegistry::GetEntitiesByTagExact(const FGameplayTag& EntityTag) const
{
    TArray<USyEntityComponent*> Result;
    
    // 直接从TagMap中获取
    const FSyEntityTagIndex* TagIndex = EntityTagMap.Find(EntityTag);
    if (TagIndex)
    {
        for (const TObjectPtr<USyEntityComponent>& Entity : TagIndex->Entities)
        {
            if (Entity)
            {
                Result.Add(Entity);
            }
        }
    }
    
    return Result;
}

TArray<USyEntityComponent*> USyEntityRegistry::GetAllRegisteredEntities() const
{
    TArray<USyEntityComponent*> Result;
    
    // 遍历ID索引，收集所有实体
    for (const auto& Pair : EntityIdMap)
    {
        if (Pair.Value)
        {
            Result.Add(Pair.Value);
        }
    }
    
    return Result;
}

void USyEntityRegistry::AddEntityToTagMap(USyEntityComponent* EntityComponent)
{
    if (!EntityComponent)
    {
        return;
    }
    
    // 获取实体的所有Tag
    FGameplayTagContainer EntityTags = EntityComponent->GetEntityTags();
    
    // 将实体添加到每个Tag对应的列表中
    for (const FGameplayTag& Tag : EntityTags)
    {
        FSyEntityTagIndex& TagIndex = EntityTagMap.FindOrAdd(Tag);
        TagIndex.Entities.AddUnique(EntityComponent);
    }
}

void USyEntityRegistry::RemoveEntityFromTagMap(USyEntityComponent* EntityComponent)
{
    if (!EntityComponent)
    {
        return;
    }
    
    // 获取实体的所有Tag
    FGameplayTagContainer EntityTags = EntityComponent->GetEntityTags();
    
    // 从每个Tag对应的列表中移除实体
    for (const FGameplayTag& Tag : EntityTags)
    {
        FSyEntityTagIndex* TagIndex = EntityTagMap.Find(Tag);
        if (TagIndex)
        {
            TagIndex->Entities.Remove(EntityComponent);
            
            // 如果列表为空，则移除整个Tag条目
            if (TagIndex->Entities.Num() == 0)
            {
                EntityTagMap.Remove(Tag);
            }
        }
    }
} 
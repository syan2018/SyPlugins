#include "Entity/SyEntityRegistry.h"
#include "Entity/SyEntityComponent.h"
#include "Entity/SyIdentityComponent.h"

namespace SyEntityRegistry::Private
{
	/**
	 * @brief 将 Tag 及其父Tag链（按 '.' 分段）展开为一组已定义的 GameplayTag。
	 * @note 为了避免依赖引擎内部父Tag遍历API的版本差异，这里用字符串前缀构造并通过 RequestGameplayTag 校验是否已定义。
	 */
	static void AppendTagAndParentsIfDefined(const FGameplayTag& Tag, TArray<FGameplayTag>& InOutTags)
	{
		if (!Tag.IsValid())
		{
			return;
		}

		const FString TagStr = Tag.ToString();
		TArray<FString> Parts;
		TagStr.ParseIntoArray(Parts, TEXT("."), /*bCullEmpty*/ true);

		FString Current;
		for (int32 i = 0; i < Parts.Num(); ++i)
		{
			Current = (i == 0) ? Parts[0] : (Current + TEXT(".") + Parts[i]);
			const FGameplayTag Candidate = FGameplayTag::RequestGameplayTag(FName(*Current), /*ErrorIfNotFound*/ false);
			if (Candidate.IsValid())
			{
				InOutTags.AddUnique(Candidate);
			}
		}
	}
}

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

	// 添加到Alias索引（可选）
	const FName Alias = EntityComponent->GetEntityAlias();
	if (Alias != NAME_None)
	{
		if (const TObjectPtr<USyEntityComponent>* Existing = EntityAliasMap.Find(Alias))
		{
			if (Existing->Get() != nullptr && Existing->Get() != EntityComponent)
			{
				UE_LOG(LogTemp, Warning, TEXT("[SyEntityRegistry] Duplicate alias '%s'. Overwriting previous entity."), *Alias.ToString());
			}
		}
		EntityAliasMap.Add(Alias, EntityComponent);
	}
    
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

	// 从Alias索引中移除
	const FName Alias = EntityComponent->GetEntityAlias();
	if (Alias != NAME_None)
	{
		const TObjectPtr<USyEntityComponent>* Existing = EntityAliasMap.Find(Alias);
		if (Existing && Existing->Get() == EntityComponent)
		{
			EntityAliasMap.Remove(Alias);
		}
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

USyEntityComponent* USyEntityRegistry::GetEntityByAlias(FName EntityAlias) const
{
	if (EntityAlias == NAME_None)
	{
		return nullptr;
	}

	if (const TObjectPtr<USyEntityComponent>* Found = EntityAliasMap.Find(EntityAlias))
	{
		return Found->Get();
	}

	return nullptr;
}

TArray<USyEntityComponent*> USyEntityRegistry::GetEntitiesByTag(const FGameplayTag& EntityTag) const
{
	// 由于注册时已把父Tag链写入索引，这里直接走索引命中即可满足 HasTag 语义。
	return GetEntitiesByTagExact(EntityTag);
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
    const FGameplayTagContainer EntityTags = EntityComponent->GetEntityTags();

	TArray<FGameplayTag> TagsToIndex;
	for (const FGameplayTag& Tag : EntityTags)
	{
		SyEntityRegistry::Private::AppendTagAndParentsIfDefined(Tag, TagsToIndex);
	}

	// 将实体添加到每个Tag（含父Tag）对应的列表中
	for (const FGameplayTag& Tag : TagsToIndex)
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
    const FGameplayTagContainer EntityTags = EntityComponent->GetEntityTags();

	TArray<FGameplayTag> TagsToUnindex;
	for (const FGameplayTag& Tag : EntityTags)
	{
		SyEntityRegistry::Private::AppendTagAndParentsIfDefined(Tag, TagsToUnindex);
	}

	// 从每个Tag（含父Tag）对应的列表中移除实体
	for (const FGameplayTag& Tag : TagsToUnindex)
	{
		FSyEntityTagIndex* TagIndex = EntityTagMap.Find(Tag);
		if (!TagIndex)
		{
			continue;
		}

		TagIndex->Entities.Remove(EntityComponent);
		if (TagIndex->Entities.Num() == 0)
		{
			EntityTagMap.Remove(Tag);
		}
	}
}

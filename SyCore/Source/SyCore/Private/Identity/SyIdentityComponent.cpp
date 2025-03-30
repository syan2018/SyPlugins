#include "Identity/SyIdentityComponent.h"
#include "Net/UnrealNetwork.h"

USyIdentityComponent::USyIdentityComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bShowDebugInfo = false;
    SetIsReplicatedByDefault(true);

    // 在构造函数中生成唯一ID
    GenerateEntityId();
}

void USyIdentityComponent::BeginPlay()
{
    Super::BeginPlay();

    // 应用默认标签
    EntityTags.AppendTags(DefaultTags);

    // 如果没有设置别名，使用ID的前8位作为默认别名
    if (EntityAlias.IsNone())
    {
        FString DefaultAlias = EntityId.ToString().Left(8);
        SetEntityAlias(*DefaultAlias);
    }
}

void USyIdentityComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    // 清理工作
}

void USyIdentityComponent::GenerateEntityId()
{
    EntityId = FGuid::NewGuid();
}

void USyIdentityComponent::SetEntityAlias(const FName& NewAlias)
{
    if (NewAlias != EntityAlias && IsAliasUnique(NewAlias))
    {
        EntityAlias = NewAlias;
        OnIdentityChanged.Broadcast(this);
    }
}

void USyIdentityComponent::AddEntityTag(const FGameplayTag& NewTag)
{
    if (!EntityTags.HasTag(NewTag))
    {
        EntityTags.AddTag(NewTag);
        OnIdentityChanged.Broadcast(this);
    }
}

void USyIdentityComponent::RemoveEntityTag(const FGameplayTag& TagToRemove)
{
    if (EntityTags.HasTag(TagToRemove))
    {
        EntityTags.RemoveTag(TagToRemove);
        OnIdentityChanged.Broadcast(this);
    }
}

bool USyIdentityComponent::HasTag(const FGameplayTag& Tag) const
{
    return EntityTags.HasTag(Tag);
}

bool USyIdentityComponent::HasAnyTags(const FGameplayTagContainer& Tags) const
{
    return EntityTags.HasAny(Tags);
}

bool USyIdentityComponent::HasAllTags(const FGameplayTagContainer& Tags) const
{
    return EntityTags.HasAll(Tags);
}

bool USyIdentityComponent::IsAliasUnique(const FName& Alias) const
{
    // TODO: 实现全局别名唯一性检查
    // 可以通过GameInstance或专门的IdentityManager来管理
    return true;
}

#include "Entity/SyIdentityComponent.h"
#include "Foundation/SyLogging.h"

USyIdentityComponent::USyIdentityComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void USyIdentityComponent::BeginPlay()
{
    Super::BeginPlay();

    // 玄学初始化，访问信息确保加载
    UE_LOG(LogSyEntity, Log, TEXT("Identity Info - ID: %s, Tags: %s, Alias: %s"),
            *GetEntityId().ToString(),
            *GetEntityTags().ToString(),
            *GetEntityAlias().ToString());
}

void USyIdentityComponent::GenerateEntityId()
{
    if (!EntityId.IsValid())
    {
        EntityId = FGuid::NewGuid();
        OnEntityIdGenerated.Broadcast();
    }
}

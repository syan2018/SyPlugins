#include "Identity/SyIdentityComponent.h"

USyIdentityComponent::USyIdentityComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void USyIdentityComponent::BeginPlay()
{
    Super::BeginPlay();
}


void USyIdentityComponent::GenerateEntityId()
{
    if (!EntityId.IsValid())
    {
        EntityId = FGuid::NewGuid();
        OnEntityIdGenerated.Broadcast();
    }
}


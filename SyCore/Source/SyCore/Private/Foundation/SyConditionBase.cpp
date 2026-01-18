#include "Foundation/SyConditionBase.h"
#include "Engine/World.h"

bool UOSyConditionBase::IsConditionMet_Implementation()
{
    return true;
}

UWorld* UOSyConditionBase::GetWorld() const
{
    if(!GEngine->GameViewport)
    {
        return nullptr;
    }
	
    return GEngine->GameViewport->GetWorld();
}

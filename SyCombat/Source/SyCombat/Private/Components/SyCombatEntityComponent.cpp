// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/SyCombatEntityComponent.h"

#include "Entity/SyEntityComponent.h"

USyCombatEntityComponent::USyCombatEntityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USyCombatEntityComponent::OnSyComponentInitialized()
{
	EntityComponent = GetOwner() ? GetOwner()->FindComponentByClass<USyEntityComponent>() : nullptr;
}

FGuid USyCombatEntityComponent::GetEntityId() const
{
	return EntityComponent ? EntityComponent->GetEntityId() : FGuid();
}

FGuid USyCombatEntityComponent::GetCombatEntityId_Implementation() const
{
	return GetEntityId();
}

FGameplayTagContainer USyCombatEntityComponent::GetCombatTags_Implementation() const
{
	return EntityComponent ? EntityComponent->GetEntityTags() : FGameplayTagContainer();
}


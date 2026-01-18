// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/SyCombatLyraPresentationBridgeComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayCueParameters.h"

USyCombatLyraPresentationBridgeComponent::USyCombatLyraPresentationBridgeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UAbilitySystemComponent* USyCombatLyraPresentationBridgeComponent::ResolveASC() const
{
	if (!GetOwner())
	{
		return nullptr;
	}

	if (GetOwner()->GetClass()->ImplementsInterface(UAbilitySystemInterface::StaticClass()))
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
		{
			return ASI->GetAbilitySystemComponent();
		}
	}

	return GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
}

void USyCombatLyraPresentationBridgeComponent::BroadcastPresentationEvent_Implementation(FGameplayTag EventTag, const FVector& ContextLocation, AActor* SourceActor)
{
	if (!EventTag.IsValid())
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = ResolveASC())
	{
		FGameplayCueParameters Params;
		Params.Location = ContextLocation;
		Params.SourceObject = SourceActor;
		ASC->ExecuteGameplayCue(EventTag, Params);
	}
}


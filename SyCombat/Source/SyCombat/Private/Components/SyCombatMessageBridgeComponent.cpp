// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/SyCombatMessageBridgeComponent.h"

#include "Messaging/SyMessageComponent.h"

USyCombatMessageBridgeComponent::USyCombatMessageBridgeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USyCombatMessageBridgeComponent::OnSyComponentInitialized()
{
	MessageComponent = GetOwner() ? GetOwner()->FindComponentByClass<USyMessageComponent>() : nullptr;
}

void USyCombatMessageBridgeComponent::BroadcastPresentationEvent_Implementation(FGameplayTag EventTag, const FVector& ContextLocation, AActor* SourceActor)
{
	if (!MessageComponent || !EventTag.IsValid())
	{
		return;
	}

	TMap<FName, FString> Metadata;
	Metadata.Add(TEXT("Location"), ContextLocation.ToString());
	if (SourceActor)
	{
		Metadata.Add(TEXT("SourceActor"), SourceActor->GetName());
	}

	MessageComponent->SendMessageWithMetadata(EventTag, Metadata);
}


// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/SyCombatInputBufferComponent.h"

USyCombatInputBufferComponent::USyCombatInputBufferComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USyCombatInputBufferComponent::BufferAction(const FSyCombatActionRequest& Request)
{
	CleanupExpired();

	if (MaxBufferSize > 0 && Buffer.Num() >= MaxBufferSize)
	{
		// 丢弃最早的输入，保证最新输入优先
		Buffer.RemoveAt(0);
	}

	FSyCombatBufferedAction Item;
	Item.Request = Request;
	Item.ExpireAtSeconds = GetWorld() ? (GetWorld()->GetTimeSeconds() + DefaultExpirationSeconds) : 0.0f;
	Buffer.Add(Item);

	OnActionBuffered.Broadcast(Request);
}

bool USyCombatInputBufferComponent::ConsumeNextBufferedAction(FSyCombatActionRequest& OutRequest)
{
	CleanupExpired();

	if (Buffer.Num() == 0)
	{
		return false;
	}

	OutRequest = Buffer[0].Request;
	Buffer.RemoveAt(0);
	return true;
}

void USyCombatInputBufferComponent::CleanupExpired()
{
	if (!GetWorld())
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	Buffer.RemoveAll([Now](const FSyCombatBufferedAction& Item)
	{
		return Item.ExpireAtSeconds > 0.0f && Item.ExpireAtSeconds <= Now;
	});
}


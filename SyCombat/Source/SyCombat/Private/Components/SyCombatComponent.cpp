// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/SyCombatComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Entity/SyEntityComponent.h"

USyCombatComponent::USyCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USyCombatComponent::OnSyComponentInitialized()
{
	EntityComponent = GetOwner() ? GetOwner()->FindComponentByClass<USyEntityComponent>() : nullptr;
}

void USyCombatComponent::RequestAction(const FSyCombatActionRequest& Request)
{
	OnActionRequested.Broadcast(Request);
}

void USyCombatComponent::ReportHit(const FSyCombatHitContext& Hit)
{
	OnHitReported.Broadcast(Hit);
}

void USyCombatComponent::BufferAction(const FSyCombatActionRequest& Request)
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

bool USyCombatComponent::ConsumeNextBufferedAction(FSyCombatActionRequest& OutRequest)
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

void USyCombatComponent::RegisterProcessor(UObject* Processor)
{
	if (!Processor || !Processor->GetClass()->ImplementsInterface(USyCombatProcessorInterface::StaticClass()))
	{
		return;
	}

	TScriptInterface<ISyCombatProcessorInterface> InterfaceObj;
	InterfaceObj.SetObject(Processor);
	InterfaceObj.SetInterface(Cast<ISyCombatProcessorInterface>(Processor));

	Processors.AddUnique(InterfaceObj);
	SortProcessors();
}

void USyCombatComponent::ClearProcessors()
{
	Processors.Reset();
}

void USyCombatComponent::ExecuteChain(FSyCombatOperationRequest& InOutRequest)
{
	for (const TScriptInterface<ISyCombatProcessorInterface>& Proc : Processors)
	{
		if (!Proc.GetObject())
		{
			continue;
		}
		ISyCombatProcessorInterface::Execute_ProcessRequest(Proc.GetObject(), InOutRequest);
	}
}

FGuid USyCombatComponent::GetCombatEntityId_Implementation() const
{
	return EntityComponent ? EntityComponent->GetEntityId() : FGuid();
}

FGameplayTagContainer USyCombatComponent::GetCombatTags_Implementation() const
{
	return EntityComponent ? EntityComponent->GetEntityTags() : FGameplayTagContainer();
}

FVector USyCombatComponent::GetTargetingPoint_Implementation(FGameplayTag BoneTag) const
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return FVector::ZeroVector;
	}

	if (USkeletalMeshComponent* SkelComp = OwnerActor->FindComponentByClass<USkeletalMeshComponent>())
	{
		if (const FName* SocketName = TargetingSocketMap.Find(BoneTag))
		{
			if (SkelComp->DoesSocketExist(*SocketName))
			{
				return SkelComp->GetSocketLocation(*SocketName);
			}
		}
	}

	return OwnerActor->GetActorLocation();
}

void USyCombatComponent::CleanupExpired()
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

void USyCombatComponent::SortProcessors()
{
	Processors.Sort([](const TScriptInterface<ISyCombatProcessorInterface>& A, const TScriptInterface<ISyCombatProcessorInterface>& B)
	{
		const int32 PA = A.GetObject() ? ISyCombatProcessorInterface::Execute_GetProcessingPriority(A.GetObject()) : 0;
		const int32 PB = B.GetObject() ? ISyCombatProcessorInterface::Execute_GetProcessingPriority(B.GetObject()) : 0;
		return PA > PB;
	});
}


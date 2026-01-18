// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/SyCombatGASAbilityDriverComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY_STATIC(LogSyCombatGASDriver, Log, All);

USyCombatGASAbilityDriverComponent::USyCombatGASAbilityDriverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USyCombatGASAbilityDriverComponent::OnSyComponentInitialized()
{
	Pipeline = GetOwner() ? GetOwner()->FindComponentByClass<USyCombatPipelineComponent>() : nullptr;
	if (!Pipeline)
	{
		UE_LOG(LogSyCombatGASDriver, Warning, TEXT("Missing USyCombatPipelineComponent on owner %s."), *GetNameSafe(GetOwner()));
		return;
	}

	// 绑定 SyCombat 的 Action 入口
	Pipeline->OnActionRequested.AddDynamic(this, &USyCombatGASAbilityDriverComponent::HandleActionRequested);
}

UAbilitySystemComponent* USyCombatGASAbilityDriverComponent::ResolveASC() const
{
	if (!GetOwner())
	{
		return nullptr;
	}

	// 优先接口
	if (GetOwner()->GetClass()->ImplementsInterface(UAbilitySystemInterface::StaticClass()))
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner()))
		{
			return ASI->GetAbilitySystemComponent();
		}
	}

	return GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
}

void USyCombatGASAbilityDriverComponent::HandleActionRequested(const FSyCombatActionRequest& Request)
{
	HandleActionRequest(Request);
}

void USyCombatGASAbilityDriverComponent::HandleActionRequest_Implementation(const FSyCombatActionRequest& Request)
{
	if (!Request.ActionTag.IsValid())
	{
		return;
	}

	const FGameplayTag* EventTagPtr = ActionToGameplayEventTag.Find(Request.ActionTag);
	if (!EventTagPtr || !EventTagPtr->IsValid())
	{
		UE_LOG(LogSyCombatGASDriver, Verbose, TEXT("No GameplayEvent mapping for ActionTag=%s"), *Request.ActionTag.ToString());
		return;
	}

	if (UAbilitySystemComponent* ASC = ResolveASC())
	{
		// 发送 GameplayEvent：Ability 可通过 TriggerTag/事件监听来预测激活
		// Payload（InstancedStruct）在此阶段先不展开；后续可在 Adapter 中定义统一的 EventData（TargetData/Direction等）
		FGameplayEventData EventData;
		EventData.EventTag = *EventTagPtr;
		EventData.Instigator = GetOwner();
		EventData.Target = GetOwner();

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), *EventTagPtr, EventData);
	}
}


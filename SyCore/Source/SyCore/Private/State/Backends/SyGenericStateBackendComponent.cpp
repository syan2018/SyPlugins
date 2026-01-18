// Copyright Epic Games, Inc. All Rights Reserved.

#include "State/Backends/SyGenericStateBackendComponent.h"

#include "State/SyStateComponent.h"
#include "State/SyStateManagerSubsystem.h"
#include "State/Operations/OperationTypes.h"
#include "State/Types/StateParameterTypes.h"

#include "Engine/World.h"
#include "GameFramework/GameInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogSyGenericStateBackend, Log, All);

USyGenericStateBackendComponent::USyGenericStateBackendComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USyGenericStateBackendComponent::OnSyComponentInitialized()
{
	StateComponent = GetOwner() ? GetOwner()->FindComponentByClass<USyStateComponent>() : nullptr;

	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			StateManager = GameInstance->GetSubsystem<USyStateManagerSubsystem>();
		}
	}
}

bool USyGenericStateBackendComponent::CanHandleChange_Implementation(const FSyStateChangeRequest& Request) const
{
	if (Request.Layer == ESyStateWriteLayer::Temporary && Request.Scope == ESyStateScope::Entity)
	{
		return StateComponent != nullptr;
	}

	if (Request.Layer == ESyStateWriteLayer::Persistent &&
		(Request.Scope == ESyStateScope::Type || Request.Scope == ESyStateScope::World))
	{
		return StateManager != nullptr;
	}

	return false;
}

bool USyGenericStateBackendComponent::ApplyChange_Implementation(const FSyStateChangeRequest& Request)
{
	if (!Request.StateTag.IsValid() || !Request.Value.IsValid())
	{
		return false;
	}

	// 1) Entity + Temporary -> StateComponent 临时层
	if (Request.Layer == ESyStateWriteLayer::Temporary && Request.Scope == ESyStateScope::Entity)
	{
		if (!StateComponent)
		{
			return false;
		}

		FSyStateParameterSet Temp;
		Temp.AddStateParam(Request.StateTag, Request.Value);
		StateComponent->ApplyTemporaryModifications(Temp);
		return true;
	}

	// 2) Type/World + Persistent -> StateManager
	if (Request.Layer == ESyStateWriteLayer::Persistent &&
		(Request.Scope == ESyStateScope::Type || Request.Scope == ESyStateScope::World))
	{
		if (!StateManager)
		{
			return false;
		}

		if (!Request.TargetTypeTag.IsValid())
		{
			UE_LOG(LogSyGenericStateBackend, Warning, TEXT("Persistent write requires TargetTypeTag for Type/World scope."));
			return false;
		}

		FSyStateParameterSet Mods;
		Mods.AddStateParam(Request.StateTag, Request.Value);

		FSyOperation Op(FSyOperationSource(Request.SourceSystemTag), FSyOperationModifier(Mods), FSyOperationTarget(Request.TargetTypeTag));
		Op.Target.TargetEntityId = Request.TargetEntityId; // 预留字段

		return StateManager->RecordOperation(Op);
	}

	return false;
}

bool USyGenericStateBackendComponent::TryGetValueStruct_Implementation(const FGameplayTag& StateTag, FInstancedStruct& OutValue) const
{
	if (!StateComponent)
	{
		return false;
	}

	return StateComponent->GetEffectiveStateParam(StateTag, OutValue);
}


// Copyright Epic Games, Inc. All Rights Reserved.

#include "State/Backends/SyGenericStateBackend.h"

#include "State/SyStateComponent.h"
#include "State/SyStateManagerSubsystem.h"
#include "State/Operations/OperationTypes.h"
#include "State/Types/StateParameterTypes.h"

#include "Engine/World.h"
#include "GameFramework/GameInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogSyGenericStateBackend, Log, All);

void USyGenericStateBackend::InitializeBackend(USyStateComponent* InOwner)
{
	Super::InitializeBackend(InOwner);

	if (UWorld* World = InOwner ? InOwner->GetWorld() : nullptr)
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			StateManager = GameInstance->GetSubsystem<USyStateManagerSubsystem>();
		}
	}
}

bool USyGenericStateBackend::CanHandleChange_Implementation(const FSyStateChangeRequest& Request) const
{
	if (Request.Layer == ESyStateWriteLayer::Temporary && Request.Scope == ESyStateScope::Entity)
	{
		return OwnerStateComponent != nullptr;
	}

	if (Request.Layer == ESyStateWriteLayer::Persistent &&
		(Request.Scope == ESyStateScope::Type || Request.Scope == ESyStateScope::World))
	{
		return StateManager != nullptr;
	}

	return false;
}

bool USyGenericStateBackend::ApplyChange_Implementation(const FSyStateChangeRequest& Request)
{
	if (!Request.StateTag.IsValid() || !Request.Value.IsValid())
	{
		return false;
	}

	// 1) Entity + Temporary -> StateComponent 临时层
	if (Request.Layer == ESyStateWriteLayer::Temporary && Request.Scope == ESyStateScope::Entity)
	{
		if (!OwnerStateComponent)
		{
			return false;
		}

		FSyStateParameterSet Temp;
		Temp.AddStateParam(Request.StateTag, Request.Value);
		OwnerStateComponent->ApplyTemporaryModifications(Temp);
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

bool USyGenericStateBackend::TryGetValueStruct_Implementation(const FGameplayTag& StateTag, FInstancedStruct& OutValue) const
{
	if (!OwnerStateComponent)
	{
		return false;
	}

	return OwnerStateComponent->GetEffectiveStateParam(StateTag, OutValue);
}


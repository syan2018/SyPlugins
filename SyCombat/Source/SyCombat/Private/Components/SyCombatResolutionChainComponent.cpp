// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/SyCombatResolutionChainComponent.h"

USyCombatResolutionChainComponent::USyCombatResolutionChainComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USyCombatResolutionChainComponent::RegisterProcessor(UObject* Processor)
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

void USyCombatResolutionChainComponent::ClearProcessors()
{
	Processors.Reset();
}

void USyCombatResolutionChainComponent::ExecuteChain(FSyCombatOperationRequest& InOutRequest)
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

void USyCombatResolutionChainComponent::SortProcessors()
{
	Processors.Sort([](const TScriptInterface<ISyCombatProcessorInterface>& A, const TScriptInterface<ISyCombatProcessorInterface>& B)
	{
		const int32 PA = A.GetObject() ? ISyCombatProcessorInterface::Execute_GetProcessingPriority(A.GetObject()) : 0;
		const int32 PB = B.GetObject() ? ISyCombatProcessorInterface::Execute_GetProcessingPriority(B.GetObject()) : 0;
		return PA > PB;
	});
}


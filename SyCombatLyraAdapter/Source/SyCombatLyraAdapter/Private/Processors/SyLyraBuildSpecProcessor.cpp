// Copyright Epic Games, Inc. All Rights Reserved.

#include "Processors/SyLyraBuildSpecProcessor.h"

#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogSyLyraBuildSpecProcessor, Log, All);

void USyLyraBuildSpecProcessor::ProcessRequest_Implementation(FSyCombatOperationRequest& InOutRequest)
{
	UE_LOG(LogSyLyraBuildSpecProcessor, VeryVerbose, TEXT("BuildSpecProcessor: Instigator=%s Target=%s"),
		*InOutRequest.InstigatorEntityId.ToString(), *InOutRequest.TargetEntityId.ToString());
}


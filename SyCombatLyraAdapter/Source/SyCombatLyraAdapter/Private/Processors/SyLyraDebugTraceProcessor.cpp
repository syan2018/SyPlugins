// Copyright Epic Games, Inc. All Rights Reserved.

#include "Processors/SyLyraDebugTraceProcessor.h"

#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogSyLyraDebugTraceProcessor, Log, All);

void USyLyraDebugTraceProcessor::ProcessRequest_Implementation(FSyCombatOperationRequest& InOutRequest)
{
	UE_LOG(LogSyLyraDebugTraceProcessor, VeryVerbose, TEXT("DebugTrace: Instigator=%s Target=%s"),
		*InOutRequest.InstigatorEntityId.ToString(), *InOutRequest.TargetEntityId.ToString());
}


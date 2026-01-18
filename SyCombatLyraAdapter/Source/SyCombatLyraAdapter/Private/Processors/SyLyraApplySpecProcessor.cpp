// Copyright Epic Games, Inc. All Rights Reserved.

#include "Processors/SyLyraApplySpecProcessor.h"

#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogSyLyraApplySpecProcessor, Log, All);

void USyLyraApplySpecProcessor::ProcessRequest_Implementation(FSyCombatOperationRequest& InOutRequest)
{
	UE_LOG(LogSyLyraApplySpecProcessor, VeryVerbose, TEXT("ApplySpecProcessor: Modifiers=%d"), InOutRequest.NumericModifiers.Num());
}


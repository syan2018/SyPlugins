// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/SyCombatPipelineComponent.h"

USyCombatPipelineComponent::USyCombatPipelineComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USyCombatPipelineComponent::RequestAction(const FSyCombatActionRequest& Request)
{
	OnActionRequested.Broadcast(Request);
}

void USyCombatPipelineComponent::ReportHit(const FSyCombatHitContext& Hit)
{
	OnHitReported.Broadcast(Hit);
}


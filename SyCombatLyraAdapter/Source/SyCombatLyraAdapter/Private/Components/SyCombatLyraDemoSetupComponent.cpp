// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/SyCombatLyraDemoSetupComponent.h"

#include "Components/SyCombatPipelineComponent.h"
#include "Components/SyCombatResolutionChainComponent.h"
#include "Components/SyCombatGASAbilityDriverComponent.h"
#include "Processors/SyLyraApplySpecProcessor.h"
#include "Processors/SyLyraBuildSpecProcessor.h"
#include "Processors/SyLyraDebugTraceProcessor.h"

#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogSyCombatLyraDemo, Log, All);

USyCombatLyraDemoSetupComponent::USyCombatLyraDemoSetupComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USyCombatLyraDemoSetupComponent::BeginPlay()
{
	Super::BeginPlay();

	USyCombatPipelineComponent* Pipeline = GetOwner() ? GetOwner()->FindComponentByClass<USyCombatPipelineComponent>() : nullptr;
	USyCombatResolutionChainComponent* Chain = GetOwner() ? GetOwner()->FindComponentByClass<USyCombatResolutionChainComponent>() : nullptr;
	USyCombatGASAbilityDriverComponent* Driver = GetOwner() ? GetOwner()->FindComponentByClass<USyCombatGASAbilityDriverComponent>() : nullptr;

	if (!Pipeline || !Chain || !Driver)
	{
		UE_LOG(LogSyCombatLyraDemo, Warning, TEXT("DemoSetup missing components on %s (Pipeline=%d, Chain=%d, Driver=%d)"),
			*GetNameSafe(GetOwner()), Pipeline != nullptr, Chain != nullptr, Driver != nullptr);
		return;
	}

	RegisterDefaultProcessors(Chain);
	ApplyDefaultActionMapping(Driver);

	UE_LOG(LogSyCombatLyraDemo, Log, TEXT("SyCombat Lyra demo setup complete for %s"), *GetNameSafe(GetOwner()));
}

void USyCombatLyraDemoSetupComponent::RegisterDefaultProcessors(USyCombatResolutionChainComponent* Chain) const
{
	if (!Chain)
	{
		return;
	}

	Chain->RegisterProcessor(NewObject<USyLyraBuildSpecProcessor>(this));
	Chain->RegisterProcessor(NewObject<USyLyraApplySpecProcessor>(this));
	Chain->RegisterProcessor(NewObject<USyLyraDebugTraceProcessor>(this));
}

void USyCombatLyraDemoSetupComponent::ApplyDefaultActionMapping(USyCombatGASAbilityDriverComponent* Driver) const
{
	if (!Driver)
	{
		return;
	}

	for (const auto& Pair : DefaultActionToEvent)
	{
		Driver->ActionToGameplayEventTag.Add(Pair.Key, Pair.Value);
	}
}


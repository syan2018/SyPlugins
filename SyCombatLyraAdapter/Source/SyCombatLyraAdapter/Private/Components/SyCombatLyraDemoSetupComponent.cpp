// Copyright Epic Games, Inc. All Rights Reserved.

#include "Components/SyCombatLyraDemoSetupComponent.h"

#include "Components/SyCombatComponent.h"
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

	USyCombatComponent* Combat = GetOwner() ? GetOwner()->FindComponentByClass<USyCombatComponent>() : nullptr;
	USyCombatGASAbilityDriverComponent* Driver = GetOwner() ? GetOwner()->FindComponentByClass<USyCombatGASAbilityDriverComponent>() : nullptr;

	if (!Combat || !Driver)
	{
		UE_LOG(LogSyCombatLyraDemo, Warning, TEXT("DemoSetup missing components on %s (Combat=%d, Driver=%d)"),
			*GetNameSafe(GetOwner()), Combat != nullptr, Driver != nullptr);
		return;
	}

	RegisterDefaultProcessors(Combat);
	ApplyDefaultActionMapping(Driver);

	UE_LOG(LogSyCombatLyraDemo, Log, TEXT("SyCombat Lyra demo setup complete for %s"), *GetNameSafe(GetOwner()));
}

void USyCombatLyraDemoSetupComponent::RegisterDefaultProcessors(USyCombatComponent* Combat)
{
	if (!Combat)
	{
		return;
	}

	Combat->RegisterProcessor(NewObject<USyLyraBuildSpecProcessor>(this));
	Combat->RegisterProcessor(NewObject<USyLyraApplySpecProcessor>(this));
	Combat->RegisterProcessor(NewObject<USyLyraDebugTraceProcessor>(this));
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


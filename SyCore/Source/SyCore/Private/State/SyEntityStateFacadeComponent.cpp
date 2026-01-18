// Copyright Epic Games, Inc. All Rights Reserved.

#include "State/SyEntityStateFacadeComponent.h"

#include "Entity/SyEntityComponent.h"
#include "Entity/SyEntityRegistry.h"
#include "State/Backends/SyStateBackendBaseComponent.h"

#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY_STATIC(LogSyStateFacade, Log, All);

USyEntityStateFacadeComponent::USyEntityStateFacadeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USyEntityStateFacadeComponent::OnSyComponentInitialized()
{
	// 统一初始化入口：由 USyEntityComponent 按 Phase 调用
	EntityComponent = GetOwner() ? GetOwner()->FindComponentByClass<USyEntityComponent>() : nullptr;

	RefreshBackendCache();
}

void USyEntityStateFacadeComponent::RefreshBackendCache()
{
	BackendComponents.Reset();

	if (!GetOwner())
	{
		return;
	}

	TArray<USyStateBackendBaseComponent*> Found;
	GetOwner()->GetComponents<USyStateBackendBaseComponent>(Found);
	for (USyStateBackendBaseComponent* C : Found)
	{
		if (C)
		{
			BackendComponents.Add(C);
		}
	}

	BackendComponents.Sort([](const TObjectPtr<USyStateBackendBaseComponent>& A, const TObjectPtr<USyStateBackendBaseComponent>& B)
	{
		const int32 PA = A ? A->Execute_GetBackendPriority(A.Get()) : 0;
		const int32 PB = B ? B->Execute_GetBackendPriority(B.Get()) : 0;
		return PA > PB; // high priority first
	});
}

bool USyEntityStateFacadeComponent::ApplyStateChange(const FSyStateChangeRequest& Request)
{
	if (!GetWorld())
	{
		return false;
	}

	if (!EntityComponent)
	{
		UE_LOG(LogSyStateFacade, Error, TEXT("ApplyStateChange failed: missing USyEntityComponent on owner."));
		return false;
	}

	const FGuid SelfId = EntityComponent->GetEntityId();

	// 1) 如果请求指定了其它实体ID，则做路由转发（保持“唯一入口”语义）
	if (Request.TargetEntityId.IsValid() && Request.TargetEntityId != SelfId)
	{
		if (USyEntityRegistry* Registry = GetWorld()->GetSubsystem<USyEntityRegistry>())
		{
			if (USyEntityComponent* TargetEntity = Registry->GetEntityById(Request.TargetEntityId))
			{
				if (USyEntityStateFacadeComponent* TargetFacade = TargetEntity->FindSyComponent<USyEntityStateFacadeComponent>())
				{
					return TargetFacade->ApplyStateChange(Request);
				}
			}
		}

		UE_LOG(LogSyStateFacade, Warning, TEXT("ApplyStateChange: TargetEntityId not found or missing facade. TargetEntityId=%s"), *Request.TargetEntityId.ToString());
		return false;
	}

	// 2) 本实体处理：补齐 TargetEntityId（便于后端/调试）
	FSyStateChangeRequest Local = Request;
	if (!Local.TargetEntityId.IsValid())
	{
		Local.TargetEntityId = SelfId;
	}

	bool bApplied = ApplyViaBackends(Local);

	if (bApplied)
	{
		OnStateChangeApplied.Broadcast(Local);
		return true;
	}

	UE_LOG(LogSyStateFacade, Error, TEXT("ApplyStateChange failed: no backend handled the request. StateTag=%s Scope=%d Layer=%d"),
		*Local.StateTag.ToString(), (int32)Local.Scope, (int32)Local.Layer);
	return false;
}

bool USyEntityStateFacadeComponent::ApplyViaBackends(const FSyStateChangeRequest& LocalRequest)
{
	for (const TObjectPtr<USyStateBackendBaseComponent>& Backend : BackendComponents)
	{
		if (!Backend)
		{
			continue;
		}

		if (!Backend->Execute_CanHandleChange(Backend.Get(), LocalRequest))
		{
			continue;
		}

		if (Backend->Execute_ApplyChange(Backend.Get(), LocalRequest))
		{
			return true;
		}
	}

	return false;
}

bool USyEntityStateFacadeComponent::TryGetEffectiveStateParam(FGameplayTag StateTag, FInstancedStruct& OutValue) const
{
	// 1) 优先询问后端（例如 GAS 后端可能才是真正权威）
	for (const TObjectPtr<USyStateBackendBaseComponent>& Backend : BackendComponents)
	{
		if (!Backend)
		{
			continue;
		}

		if (Backend->Execute_TryGetValueStruct(Backend.Get(), StateTag, OutValue))
		{
			return true;
		}
	}

	OutValue.Reset();
	return false;
}


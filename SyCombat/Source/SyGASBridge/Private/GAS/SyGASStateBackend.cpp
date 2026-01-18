// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/SyGASStateBackend.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

#include "GAS/SyStateToGASMapping.h"
#include "State/SyStateComponent.h"

#include "State/Types/Metadatas/BasicMetadataValueTypes.h" // FSyBoolValue

DEFINE_LOG_CATEGORY_STATIC(LogSyGASBridge, Log, All);

USyGASStateBackend::USyGASStateBackend()
{
}

void USyGASStateBackend::InitializeBackend(USyStateComponent* InOwner)
{
	Super::InitializeBackend(InOwner);
	EnsureASC();
}

bool USyGASStateBackend::EnsureASC()
{
	if (ASC)
	{
		return true;
	}

	if (!OwnerStateComponent || !OwnerStateComponent->GetOwner())
	{
		return false;
	}

	AActor* OwnerActor = OwnerStateComponent->GetOwner();

	// 1) 优先使用 AbilitySystemInterface（Lyra 风格）
	if (OwnerActor->GetClass()->ImplementsInterface(UAbilitySystemInterface::StaticClass()))
	{
		IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OwnerActor);
		if (ASI)
		{
			ASC = ASI->GetAbilitySystemComponent();
		}
	}

	// 2) 回退：直接找组件
	if (!ASC)
	{
		ASC = OwnerActor->FindComponentByClass<UAbilitySystemComponent>();
	}

	if (!ASC)
	{
		UE_LOG(LogSyGASBridge, Verbose, TEXT("No ASC found on owner %s."), *GetNameSafe(OwnerActor));
		return false;
	}

	return true;
}

const FSyStateToGASTagMapping* USyGASStateBackend::FindTagMapping(const FGameplayTag& SyStateTag) const
{
	if (!Mapping || !SyStateTag.IsValid())
	{
		return nullptr;
	}

	for (const FSyStateToGASTagMapping& M : Mapping->TagMappings)
	{
		if (M.SyStateTag == SyStateTag)
		{
			return &M;
		}
	}

	return nullptr;
}

bool USyGASStateBackend::ExtractBoolValue(const FInstancedStruct& Value, bool& OutBool) const
{
	// 约定：如果 Value 缺失/无效，则视为“启用”
	if (!Value.IsValid())
	{
		OutBool = true;
		return true;
	}

	// SyCore 的基础 bool 值类型
	if (Value.GetScriptStruct() == FSyBoolValue::StaticStruct())
	{
		if (const FSyBoolValue* Ptr = Value.GetPtr<FSyBoolValue>())
		{
			OutBool = Ptr->Value;
			return true;
		}
	}

	// 兜底：无法解析则不处理
	return false;
}

bool USyGASStateBackend::CanHandleChange_Implementation(const FSyStateChangeRequest& Request) const
{
	if (Request.Scope != ESyStateScope::Entity)
	{
		return false;
	}

	if (!Request.StateTag.IsValid())
	{
		return false;
	}

	// 仅在存在映射时声明可处理（避免抢走其它后端）
	return FindTagMapping(Request.StateTag) != nullptr;
}

bool USyGASStateBackend::ApplyChange_Implementation(const FSyStateChangeRequest& Request)
{
	if (!EnsureASC())
	{
		return false;
	}

	const FSyStateToGASTagMapping* M = FindTagMapping(Request.StateTag);
	if (!M)
	{
		return false;
	}

	bool bEnable = true;
	if (!ExtractBoolValue(Request.Value, bEnable))
	{
		UE_LOG(LogSyGASBridge, Warning, TEXT("Failed to extract bool value for SyStateTag=%s"), *Request.StateTag.ToString());
		return false;
	}

	// Temporary：优先 loose tag（即时/预测）
	if (Request.Layer == ESyStateWriteLayer::Temporary)
	{
		return ApplyTagAsLoose(M->GameplayTag, bEnable);
	}

	// Persistent：优先 effect（复制/回滚友好），无 effect 再退化为 loose
	if (Request.Layer == ESyStateWriteLayer::Persistent)
	{
		if (M->PersistentTagEffect)
		{
			// 服务器权威应用GE；客户端可用 loose tag 立即反馈（由 bAllowLooseTagPrediction 控制）
			const bool bEffectApplied = ApplyTagAsEffect(M->PersistentTagEffect, Request.StateTag, bEnable);
			if (!bEffectApplied && M->bAllowLooseTagPrediction)
			{
				return ApplyTagAsLoose(M->GameplayTag, bEnable);
			}
			return bEffectApplied;
		}

		if (M->bAllowLooseTagPrediction)
		{
			return ApplyTagAsLoose(M->GameplayTag, bEnable);
		}
	}

	return false;
}

bool USyGASStateBackend::ApplyTagAsLoose(const FGameplayTag& TagToApply, bool bEnable)
{
	if (!TagToApply.IsValid() || !ASC)
	{
		return false;
	}

	if (bEnable)
	{
		ASC->AddLooseGameplayTag(TagToApply);
	}
	else
	{
		ASC->RemoveLooseGameplayTag(TagToApply);
	}

	return true;
}

bool USyGASStateBackend::ApplyTagAsEffect(const TSubclassOf<UGameplayEffect>& EffectClass, const FGameplayTag& SyStateTag, bool bEnable)
{
	if (!EffectClass || !ASC || !SyStateTag.IsValid())
	{
		return false;
	}

	// 仅在服务器权威侧应用/移除 GE（复制交给 GAS）
	if (!ASC->GetOwner() || !ASC->GetOwner()->HasAuthority())
	{
		return false;
	}

	if (!bEnable)
	{
		if (const FActiveGameplayEffectHandle* Handle = ActiveTagEffects.Find(SyStateTag))
		{
			if (Handle->IsValid())
			{
				ASC->RemoveActiveGameplayEffect(*Handle);
			}
			ActiveTagEffects.Remove(SyStateTag);
			return true;
		}
		return true; // already removed
	}

	// Enable：创建并应用到自身
	FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
	Ctx.AddSourceObject(this);

	const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, /*Level*/ 1.0f, Ctx);
	if (!SpecHandle.IsValid())
	{
		return false;
	}

	const FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	if (ActiveHandle.IsValid())
	{
		ActiveTagEffects.Add(SyStateTag, ActiveHandle);
		return true;
	}

	return false;
}


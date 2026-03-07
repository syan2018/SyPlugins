#include "Components/SyCombatLyraObserverComponent.h"

#include "Components/SyCombatComponent.h"
#include "Character/LyraHealthComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GameplayAbility.h"
#include "State/SyStateComponent.h"
#include "State/SyStateTypes.h"
#include "Entity/SyEntityComponent.h"

DEFINE_LOG_CATEGORY(LogSyCombatLyraObserver);

USyCombatLyraObserverComponent::USyCombatLyraObserverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	DeathStateTag = FGameplayTag::RequestGameplayTag(FName("Sy.State.Life.Dead"), false);
}

void USyCombatLyraObserverComponent::OnSyComponentInitialized()
{
	CombatComponent = GetOwner() ? GetOwner()->FindComponentByClass<USyCombatComponent>() : nullptr;

	BindToLyraHealth();
	BindToASC();

	UE_LOG(LogSyCombatLyraObserver, Log,
		TEXT("[%s] Observer initialized (Health=%d, ASC=%d, Combat=%d)"),
		*GetNameSafe(GetOwner()),
		HealthComponent != nullptr, ASC != nullptr, CombatComponent != nullptr);
}

void USyCombatLyraObserverComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ASC)
	{
		ASC->AbilityActivatedCallbacks.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

// --- Lyra Health 绑定 ---

void USyCombatLyraObserverComponent::BindToLyraHealth()
{
	if (!GetOwner()) return;

	HealthComponent = GetOwner()->FindComponentByClass<ULyraHealthComponent>();
	if (!HealthComponent)
	{
		UE_LOG(LogSyCombatLyraObserver, Warning,
			TEXT("[%s] ULyraHealthComponent not found, health observation disabled."),
			*GetNameSafe(GetOwner()));
		return;
	}

	HealthComponent->OnHealthChanged.AddDynamic(this, &USyCombatLyraObserverComponent::HandleLyraHealthChanged);
	HealthComponent->OnDeathStarted.AddDynamic(this, &USyCombatLyraObserverComponent::HandleLyraDeathStarted);
}

void USyCombatLyraObserverComponent::HandleLyraHealthChanged(
	ULyraHealthComponent* InHealthComp, float OldValue, float NewValue, AActor* Instigator)
{
	const float Delta = NewValue - OldValue;

	UE_LOG(LogSyCombatLyraObserver, Log,
		TEXT("[%s] Health: %.1f -> %.1f (Delta=%.1f, Instigator=%s)"),
		*GetNameSafe(GetOwner()), OldValue, NewValue, Delta, *GetNameSafe(Instigator));

	if (bFeedHitsToSyCombat && CombatComponent && Delta < 0.0f)
	{
		FSyCombatHitContext HitCtx;
		if (USyEntityComponent* InstigatorEntity = Instigator ? Instigator->FindComponentByClass<USyEntityComponent>() : nullptr)
		{
			HitCtx.InstigatorEntityId = InstigatorEntity->GetEntityId();
		}
		if (USyEntityComponent* SelfEntity = GetOwner()->FindComponentByClass<USyEntityComponent>())
		{
			HitCtx.TargetEntityId = SelfEntity->GetEntityId();
		}
		HitCtx.ContextTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Sy.Combat.Hit.Observed"), false));

		CombatComponent->ReportHit(HitCtx);
		UE_LOG(LogSyCombatLyraObserver, Verbose, TEXT("  -> ReportHit fed to SyCombat pipeline"));
	}
}

void USyCombatLyraObserverComponent::HandleLyraDeathStarted(AActor* OwningActor)
{
	UE_LOG(LogSyCombatLyraObserver, Log,
		TEXT("[%s] Death started!"), *GetNameSafe(OwningActor));

	if (bSyncDeathToSyState)
	{
		WriteSyStateDead();
	}
}

// --- ASC Ability 监听 ---

void USyCombatLyraObserverComponent::BindToASC()
{
	if (!GetOwner()) return;

	ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	if (!ASC)
	{
		UE_LOG(LogSyCombatLyraObserver, Warning,
			TEXT("[%s] AbilitySystemComponent not found, ability observation disabled."),
			*GetNameSafe(GetOwner()));
		return;
	}

	ASC->AbilityActivatedCallbacks.AddUObject(this, &USyCombatLyraObserverComponent::HandleAbilityActivated);
}

void USyCombatLyraObserverComponent::HandleAbilityActivated(UGameplayAbility* Ability)
{
	if (!Ability) return;

	UE_LOG(LogSyCombatLyraObserver, Log,
		TEXT("[%s] Ability activated: %s (Class=%s)"),
		*GetNameSafe(GetOwner()),
		*Ability->GetName(),
		*Ability->GetClass()->GetName());
}

// --- SyState 写入 ---

void USyCombatLyraObserverComponent::WriteSyStateDead()
{
	if (!DeathStateTag.IsValid())
	{
		UE_LOG(LogSyCombatLyraObserver, Warning, TEXT("DeathStateTag is not valid, skipping SyState write."));
		return;
	}

	USyStateComponent* StateComp = GetOwner() ? GetOwner()->FindComponentByClass<USyStateComponent>() : nullptr;
	if (!StateComp)
	{
		UE_LOG(LogSyCombatLyraObserver, Warning,
			TEXT("[%s] USyStateComponent not found, cannot write death state."),
			*GetNameSafe(GetOwner()));
		return;
	}

	FSyStateChangeRequest Request;
	Request.Scope = ESyStateScope::Entity;
	Request.Layer = ESyStateWriteLayer::Temporary;
	Request.StateTag = DeathStateTag;
	// Value 留空 — StateTag 存在即表示 "Dead=true"，后端按 Tag 路由处理
	Request.SourceSystemTag = FGameplayTag::RequestGameplayTag(FName("Sy.System.CombatObserver"), false);

	const bool bSuccess = StateComp->ApplyStateChange(Request);
	UE_LOG(LogSyCombatLyraObserver, Log,
		TEXT("[%s] WriteSyStateDead -> %s (Tag=%s)"),
		*GetNameSafe(GetOwner()),
		bSuccess ? TEXT("Success") : TEXT("Failed"),
		*DeathStateTag.ToString());
}

#include "Components/SyCombatLyraInputBridgeComponent.h"

#include "Components/SyCombatComponent.h"
#include "Types/SyCombatTypes.h"

#include "Character/LyraHeroComponent.h"
#include "Input/LyraInputComponent.h"
#include "Input/LyraInputConfig.h"
#include "Character/LyraPawnExtensionComponent.h"
#include "Character/LyraPawnData.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogSyCombatLyraInputBridge);

USyCombatLyraInputBridgeComponent::USyCombatLyraInputBridgeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USyCombatLyraInputBridgeComponent::OnSyComponentInitialized()
{
	CombatComponent = GetOwner() ? GetOwner()->FindComponentByClass<USyCombatComponent>() : nullptr;

	if (!CombatComponent)
	{
		UE_LOG(LogSyCombatLyraInputBridge, Warning,
			TEXT("[%s] USyCombatComponent not found, input bridge disabled."),
			*GetNameSafe(GetOwner()));
		return;
	}

	UE_LOG(LogSyCombatLyraInputBridge, Log,
		TEXT("[%s] InputBridge SyComponent initialized (Mappings=%d, TakeoverMode=%d)"),
		*GetNameSafe(GetOwner()), InputTagToActionTag.Num(), bTakeoverMode);
}

void USyCombatLyraInputBridgeComponent::BeginPlay()
{
	Super::BeginPlay();

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		return;
	}

	// IsLocallyControlled 在 BeginPlay 时 Controller 可能还没 Possess，
	// 延迟到轮询中检查
	TryBindOrScheduleRetry();
}

void USyCombatLyraInputBridgeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bTakeoverMode && bInputBound)
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
		{
			ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Gameplay.AbilityInputBlocked")));
		}
	}

	Super::EndPlay(EndPlayReason);
}

void USyCombatLyraInputBridgeComponent::TryBindOrScheduleRetry()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		return;
	}

	// AI Bot 没有 PlayerController，跳过
	if (!Pawn->IsLocallyControlled())
	{
		// 下一帧重试，等待 Controller Possess 完成
		RetryCount++;
		if (RetryCount > MaxRetryCount)
		{
			// 超过重试上限，可能是 AI Bot，静默放弃
			return;
		}

		if (UWorld* World = GetWorld())
		{
			TWeakObjectPtr<USyCombatLyraInputBridgeComponent> WeakThis(this);
			World->GetTimerManager().SetTimerForNextTick([WeakThis]()
			{
				if (WeakThis.IsValid())
				{
					WeakThis->TryBindOrScheduleRetry();
				}
			});
		}
		return;
	}

	ULyraHeroComponent* HeroComp = Pawn->FindComponentByClass<ULyraHeroComponent>();
	if (HeroComp && HeroComp->IsReadyToBindInputs())
	{
		BindAbilityInputToPawn(Pawn);
		return;
	}

	// HeroComponent 还没 Ready，下一帧重试
	RetryCount++;
	if (RetryCount > MaxRetryCount)
	{
		UE_LOG(LogSyCombatLyraInputBridge, Warning,
			TEXT("[%s] Gave up waiting for HeroComponent after %d retries."),
			*GetNameSafe(Pawn), MaxRetryCount);
		return;
	}

	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<USyCombatLyraInputBridgeComponent> WeakThis(this);
		World->GetTimerManager().SetTimerForNextTick([WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->TryBindOrScheduleRetry();
			}
		});
	}
}

void USyCombatLyraInputBridgeComponent::BindAbilityInputToPawn(APawn* Pawn)
{
	if (bInputBound || !Pawn || !CombatComponent)
	{
		return;
	}

	ULyraInputComponent* LyraIC = Pawn->FindComponentByClass<ULyraInputComponent>();
	if (!LyraIC)
	{
		UE_LOG(LogSyCombatLyraInputBridge, Warning,
			TEXT("[%s] ULyraInputComponent not found."), *GetNameSafe(Pawn));
		return;
	}

	ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(Pawn);
	if (!PawnExtComp)
	{
		return;
	}

	const ULyraPawnData* PawnData = PawnExtComp->GetPawnData<ULyraPawnData>();
	if (!PawnData || !PawnData->InputConfig)
	{
		UE_LOG(LogSyCombatLyraInputBridge, Warning,
			TEXT("[%s] No PawnData or InputConfig."), *GetNameSafe(Pawn));
		return;
	}

	// 和 LyraHeroComponent::InitializePlayerInput 相同模式：
	// 通过 BindAbilityActions 注册额外的 InputAction 回调。
	// Lyra 的 EnhancedInput 支持同一 InputAction 绑定多个回调，因此并行模式天然可行。
	TArray<uint32> BindHandles;
	LyraIC->BindAbilityActions(
		PawnData->InputConfig,
		this,
		&USyCombatLyraInputBridgeComponent::OnAbilityInputTagPressed,
		&USyCombatLyraInputBridgeComponent::OnAbilityInputTagReleased,
		BindHandles
	);

	bInputBound = true;

	UE_LOG(LogSyCombatLyraInputBridge, Log,
		TEXT("[%s] Ability input bound (%d handles, TakeoverMode=%d, InputMappings=%d)"),
		*GetNameSafe(Pawn), BindHandles.Num(), bTakeoverMode, InputTagToActionTag.Num());

	// Phase 2b 接管模式：阻止 Lyra 原生 ProcessAbilityInput
	if (bTakeoverMode)
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn))
		{
			ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("Gameplay.AbilityInputBlocked")));
			UE_LOG(LogSyCombatLyraInputBridge, Log,
				TEXT("[%s] Takeover: Gameplay.AbilityInputBlocked added."), *GetNameSafe(Pawn));
		}
	}
}

void USyCombatLyraInputBridgeComponent::OnAbilityInputTagPressed(FGameplayTag InputTag)
{
	if (!InputTag.IsValid() || !CombatComponent)
	{
		return;
	}

	const FGameplayTag* ActionTagPtr = InputTagToActionTag.Find(InputTag);
	if (!ActionTagPtr || !ActionTagPtr->IsValid())
	{
		return;
	}

	FSyCombatActionRequest Request;
	Request.ActionTag = *ActionTagPtr;

	CombatComponent->RequestAction(Request);

	UE_LOG(LogSyCombatLyraInputBridge, Log,
		TEXT("InputTag [%s] -> ActionTag [%s] -> RequestAction"),
		*InputTag.ToString(), *ActionTagPtr->ToString());
}

void USyCombatLyraInputBridgeComponent::OnAbilityInputTagReleased(FGameplayTag InputTag)
{
	// Released 留作后续扩展（长按/蓄力等）
}

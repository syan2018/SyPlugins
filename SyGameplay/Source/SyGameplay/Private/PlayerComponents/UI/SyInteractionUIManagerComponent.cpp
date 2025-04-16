#include "PlayerComponents/UI/SyInteractionUIManagerComponent.h"
#include "PlayerComponents/Manager/SyPlayerInteractionManagerComponent.h"
#include "Components/SyInteractionComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Components/TextBlock.h" // 假设Widget中有一个名为InteractionPromptText的TextBlock

USyInteractionUIManagerComponent::USyInteractionUIManagerComponent()
{
    // 通常UI管理器不需要Tick
    PrimaryComponentTick.bCanEverTick = false;
}

void USyInteractionUIManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("SyInteractionUIManagerComponent requires an owning Actor."));
        return;
    }

    // 查找Owner身上的交互管理器组件
    InteractionManager = OwnerActor->FindComponentByClass<USyPlayerInteractionManagerComponent>();

    if (InteractionManager.IsValid())
    {
        // 绑定激活交互变化的事件
        InteractionManager->OnActiveInteractionChanged.AddDynamic(this, &USyInteractionUIManagerComponent::HandleActiveInteractionChanged);

        // 立即检查一次初始状态
        HandleActiveInteractionChanged(InteractionManager->GetActiveInteraction());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SyInteractionUIManagerComponent could not find USyPlayerInteractionManagerComponent on %s."), *GetNameSafe(OwnerActor));
    }
}

void USyInteractionUIManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 隐藏并清理Widget
    HideInteractionWidget();

    // 解绑事件
    if (InteractionManager.IsValid())
    {
        InteractionManager->OnActiveInteractionChanged.RemoveDynamic(this, &USyInteractionUIManagerComponent::HandleActiveInteractionChanged);
    }

    Super::EndPlay(EndPlayReason);
}

void USyInteractionUIManagerComponent::HandleActiveInteractionChanged(USyInteractionComponent* NewActiveInteraction)
{
    if (NewActiveInteraction && NewActiveInteraction->bEnabled)
    {
        ShowInteractionWidget(NewActiveInteraction);
        // TODO: USyInteractionComponent 需要添加 GetInteractionPromptText() 方法 
        // 暂时使用默认文本
        // UpdateInteractionWidgetText(NewActiveInteraction->GetInteractionPromptText());
        UpdateInteractionWidgetText(FText::FromString(TEXT("[E] Interact")));
    }
    else
    {
        HideInteractionWidget();
    }
}

void USyInteractionUIManagerComponent::ShowInteractionWidget(USyInteractionComponent* InteractionComponent)
{
    if (!InteractionWidgetClass)
    {
        // UE_LOG(LogTemp, Warning, TEXT("InteractionWidgetClass is not set in SyInteractionUIManagerComponent."));
        return;
    }

    APlayerController* PC = GetOwner<APawn>() ? GetOwner<APawn>()->GetController<APlayerController>() : GetOwner<APlayerController>();
    if (!PC || !PC->IsLocalController())
    {
        // 如果不是本地玩家控制器，则不显示UI
        return;
    }

    if (!CurrentInteractionWidget)
    {
        CurrentInteractionWidget = CreateWidget<UUserWidget>(PC, InteractionWidgetClass);
        if (CurrentInteractionWidget)
        {
            CurrentInteractionWidget->AddToViewport();
            // UE_LOG(LogTemp, Log, TEXT("Interaction Widget Created and Added to Viewport."));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create Interaction Widget."));
        }
    }
    else if (!CurrentInteractionWidget->IsInViewport())
    {
        // 如果Widget已存在但未在视口中，则重新添加
        CurrentInteractionWidget->AddToViewport();
        // UE_LOG(LogTemp, Log, TEXT("Interaction Widget Added to Viewport again."));
    }
}

void USyInteractionUIManagerComponent::HideInteractionWidget()
{
    if (CurrentInteractionWidget)
    {
        CurrentInteractionWidget->RemoveFromParent();
        CurrentInteractionWidget = nullptr;
        // UE_LOG(LogTemp, Log, TEXT("Interaction Widget Removed from Viewport."));
    }
}

void USyInteractionUIManagerComponent::UpdateInteractionWidgetText(const FText& PromptText)
{
    if (CurrentInteractionWidget)
    {
        // 假设Widget蓝图中有一个名为 InteractionPromptText 的 TextBlock 控件
        UTextBlock* PromptTextBlock = Cast<UTextBlock>(CurrentInteractionWidget->GetWidgetFromName(TEXT("InteractionPromptText")));
        if (PromptTextBlock)
        {
            PromptTextBlock->SetText(PromptText);
        }
        else
        {
            // UE_LOG(LogTemp, Warning, TEXT("Could not find TextBlock named 'InteractionPromptText' in the Interaction Widget."));
        }
    }
} 
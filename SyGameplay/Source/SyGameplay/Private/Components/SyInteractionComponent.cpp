#include "Components/SyInteractionComponent.h"
#include "States/SyStateComponent.h"
#include "GameFramework/Actor.h"
#include "Logging/LogMacros.h"
#include "Metadatas/BasicMetadataValueTypes.h"
#include "SyStateCore/Public/StateMetadataTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogSyInteraction, Log, All);

USyInteractionComponent::USyInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bEnabled = false; // 默认禁用，等待状态更新
}

void USyInteractionComponent::BeginPlay()
{
    Super::BeginPlay();

    // 查找并缓存StateComponent
    FindAndCacheComponent();

    // 处理本地交互逻辑
    OnUsed.AddDynamic(this, &USyInteractionComponent::HandleInteractionRequest);

    // 如果找到StateComponent，绑定状态变化事件
    if (StateComponent)
    {
        StateComponent->OnLocalStateDataChanged.AddUObject(this, &USyInteractionComponent::HandleStateChanged);
        // 初始状态检查
        HandleStateChanged();
    }
    else
    {
        UE_LOG(LogSyInteraction, Warning, TEXT("%s: Could not find StateComponent on owner actor."), *GetNameSafe(GetOwner()));
    }
}

void USyInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 解绑状态变化事件
    if (StateComponent)
    {
        StateComponent->OnLocalStateDataChanged.RemoveAll(this);
    }

    Super::EndPlay(EndPlayReason);
}

void USyInteractionComponent::FindAndCacheComponent()
{
    if (!GetOwner())
    {
        return;
    }

    // 查找StateComponent
    StateComponent = GetOwner()->FindComponentByClass<USyStateComponent>();
    if (!StateComponent)
    {
        UE_LOG(LogSyInteraction, Warning, TEXT("%s: Could not find StateComponent on owner actor."), *GetNameSafe(GetOwner()));
    }

    // 查找EntityComponent
    EntityComponent = GetOwner()->FindComponentByClass<USyEntityComponent>();
    if (!EntityComponent)
    {
        UE_LOG(LogSyInteraction, Warning, TEXT("%s: Could not find EntityComponent on owner actor."), *GetNameSafe(GetOwner()));
    }
}

void USyInteractionComponent::HandleStateChanged()
{
    if (!StateComponent)
    {
        return;
    }

    // 检查"State.Interactable"标签的状态
    const FGameplayTag InteractableTag = FGameplayTag::RequestGameplayTag(TEXT("State.Interactable"));
    const FSyStateCategories& CurrentState = StateComponent->GetCurrentStateCategories();
    
    // 查找Interactable标签的第一个元数据
    if (USyStateMetadataBase* Metadata = CurrentState.FindFirstStateMetadata<USyStateMetadataBase>(InteractableTag))
    {
        // 尝试获取布尔值
        // bool bIsInteractable = false;
        FSyBoolValue bIsInteractable = false;
        if (Metadata->TryGetValue(bIsInteractable))
        {
            // 根据状态更新交互能力
            if (bIsInteractable.Value)
            {
                Enable();
                UE_LOG(LogSyInteraction, Verbose, TEXT("%s: Interaction enabled due to State.Interactable tag."), *GetNameSafe(GetOwner()));
            }
            else
            {
                Disable();
                UE_LOG(LogSyInteraction, Verbose, TEXT("%s: Interaction disabled due to State.Interactable tag."), *GetNameSafe(GetOwner()));
            }
        }
        else
        {
            UE_LOG(LogSyInteraction, Warning, TEXT("%s: State.Interactable metadata is not a boolean value."), *GetNameSafe(GetOwner()));
        }
    }
    else
    {
        // 如果没有找到Interactable标签的元数据，默认禁用
        Disable();
        UE_LOG(LogSyInteraction, Verbose, TEXT("%s: Interaction disabled due to missing State.Interactable tag."), *GetNameSafe(GetOwner()));
    }
}


void USyInteractionComponent::HandleInteractionRequest()
{
    UE_LOG(LogSyInteraction, Verbose, TEXT("%s: Interaction requested."), *GetNameSafe(GetOwner()));

    EntityComponent->SendMessage(FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction")));
    EntityComponent->SendMessage(FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Start")));
    
    // 待补完完整交互逻辑 

    // TODO: 照理说会放在通用功能执行脚本的完成回调里，后续进行设计 
    EntityComponent->SendMessage(FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.End")));
}

#include "Components/SyInteractionComponent.h"
#include "State/SyStateComponent.h"
#include "GameFramework/Actor.h"
#include "Logging/LogMacros.h"
#include "State/Types/Metadatas/BasicMetadataValueTypes.h"
#include "Metadatas/SyGameplayInteractMetadatas.h"
#include "Metadatas/SyGameplayInteractValueTypes.h"
#include "FlowComponent.h"
#include "FlowSubsystem.h"
#include "GameplayTagContainer.h"

DEFINE_LOG_CATEGORY_STATIC(LogSyInteraction, Log, All);

USyInteractionComponent::USyInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bEnabled = false;
}

void USyInteractionComponent::BeginPlay()
{
    Super::BeginPlay();

    OnUsed.AddDynamic(this, &USyInteractionComponent::HandleInteractionRequest);
}

void USyInteractionComponent::OnSyComponentInitialized()
{
    // ✅ 统一初始化入口：查找组件、绑定监听、处理初始状态
    FindAndCacheComponents();
    
    if (CachedStateComponent)
    {
        // 1. 绑定状态变化监听（监听后续变化）
        CachedStateComponent->OnEffectiveStateChanged.AddUObject(this, &USyInteractionComponent::HandleStateChanged);
        
        // 2. ✅ 主动处理初始状态（因为 StateComponent 的广播已经在 Core 阶段完成）
        HandleStateChanged();
        
        UE_LOG(LogSyInteraction, Log, TEXT("Actor %s: InteractionComponent initialized, processed initial state, and listening to state changes."), *GetNameSafe(GetOwner()));
    }
    else
    {
        UE_LOG(LogSyInteraction, Warning, TEXT("Actor %s: InteractionComponent could not find StateComponent."), *GetNameSafe(GetOwner()));
    }
}

void USyInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (CachedStateComponent)
    {
        CachedStateComponent->OnEffectiveStateChanged.RemoveAll(this);
    }

    Super::EndPlay(EndPlayReason);
}

void USyInteractionComponent::FindAndCacheComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // 先查找 EntityComponent
    CachedEntityComponent = Owner->FindComponentByClass<USyEntityComponent>();
    if (!CachedEntityComponent)
    {
        UE_LOG(LogSyInteraction, Warning, TEXT("Actor %s: InteractionComponent could not find EntityComponent."), *GetNameSafe(Owner));
        return;
    }

    // 从 EntityComponent 获取 StateComponent（确保是同一个）
    CachedStateComponent = CachedEntityComponent->GetStateComponent();
    if (CachedStateComponent)
    {
        UE_LOG(LogSyInteraction, Log, TEXT("Actor %s: InteractionComponent cached StateComponent (Address: %p, Name: %s)"), 
            *GetNameSafe(Owner), 
            CachedStateComponent.Get(),
            *CachedStateComponent->GetName());
    }
    else
    {
        UE_LOG(LogSyInteraction, Warning, TEXT("Actor %s: InteractionComponent could not get StateComponent from EntityComponent."), *GetNameSafe(Owner));
    }
}

void USyInteractionComponent::HandleStateChanged()
{
    if (!CachedStateComponent)
    {
        return;
    }

    const FGameplayTag InteractableTag = FGameplayTag::RequestGameplayTag(TEXT("State.Interact.Interactable"));
    const FSyStateCategories& CurrentState = CachedStateComponent->GetEffectiveStateCategories();
    
    if (USyStateMetadataBase* Metadata = CurrentState.FindFirstStateMetadata<USyStateMetadataBase>(InteractableTag))
    {
        FSyBoolValue bIsInteractable = false;
        if (Metadata->TryGetValue(bIsInteractable))
        {
            if (bIsInteractable.Value)
            {
                Enable();
            }
            else
            {
                Disable();
            }
        }
        else
        {
            UE_LOG(LogSyInteraction, Warning, TEXT("Actor %s: State.Interactable metadata found but is not FSyBoolValue."), *GetNameSafe(GetOwner()));
            Disable();
        }
    }
    else
    {
        Disable();
    }
}

void USyInteractionComponent::HandleInteractionRequest()
{
    UE_LOG(LogSyInteraction, Verbose, TEXT("Actor %s: Interaction requested."), *GetNameSafe(GetOwner()));

    if (!CachedStateComponent || !CachedEntityComponent)
    {
        UE_LOG(LogSyInteraction, Warning, TEXT("Interaction request ignored: StateComponent or EntityComponent is missing."));
        return;
    }

    CachedEntityComponent->BroadcastEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Start")));
    
    // 进入交互后临时关闭，肯定有更合适的处理方式
    Disable();

    const FSyStateCategories& CurrentState = CachedStateComponent->GetEffectiveStateCategories();
    const USyInteractionListMetadata* ListMetadata = CurrentState.FindFirstStateMetadata<USyInteractionListMetadata>(InteractionListMetadataTag);

    if (!ListMetadata)
    {
        UE_LOG(LogSyInteraction, Warning, TEXT("Actor %s: Interaction failed. No InteractionListMetadata found with tag '%s'."), 
            *GetNameSafe(GetOwner()), *InteractionListMetadataTag.ToString());
        CachedEntityComponent->BroadcastEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.End")));
        return;
    }

    const TArray<FInstancedStruct>& InteractionItems = ListMetadata->GetInteractionList();

    if (InteractionItems.IsEmpty())
    {
        UE_LOG(LogSyInteraction, Log, TEXT("Actor %s: Interaction list is empty."), *GetNameSafe(GetOwner()));
        CachedEntityComponent->BroadcastEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.End")));
        return;
    }

    // 待优化适配多种交互，主要使用常见为多闲聊挂接，也可以考虑通过优化对话系统实现
    const FInstancedStruct& FirstInteractionItem = InteractionItems[0];

    if (!FirstInteractionItem.IsValid() || !FirstInteractionItem.GetScriptStruct()->IsChildOf(FSyInteractInfoBase::StaticStruct()))
    {
        UE_LOG(LogSyInteraction, Error, TEXT("Actor %s: First interaction item is invalid or not derived from FSyInteractInfoBase."), *GetNameSafe(GetOwner()));
        CachedEntityComponent->BroadcastEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.End")));
        return;
    }

    const FSyInteractInfoBase* BaseInfo = FirstInteractionItem.GetPtr<FSyInteractInfoBase>();
    if(!BaseInfo)
    {
        UE_LOG(LogSyInteraction, Error, TEXT("Actor %s: Failed to get base pointer from first interaction item."), *GetNameSafe(GetOwner()));
        CachedEntityComponent->BroadcastEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.End")));
        return;
    }

    const FGameplayTag InteractionType = BaseInfo->InteractTypeTag;
    UE_LOG(LogSyInteraction, Log, TEXT("Executing interaction of type: %s"), *InteractionType.ToString());

    if (InteractionType == FGameplayTag::RequestGameplayTag(FName("Interaction.Basic")))
    {
        ProcessBasicInteraction(FirstInteractionItem);
    }
    else if (InteractionType == FGameplayTag::RequestGameplayTag(FName("Interaction.Flow")))
    {
        ProcessFlowInteraction(FirstInteractionItem);
    }
    else if (InteractionType == FGameplayTag::RequestGameplayTag(FName("Interaction.StateTree")))
    {
        UE_LOG(LogSyInteraction, Warning, TEXT("StateTree interaction type is not yet implemented."));
    }
    else
    {
        UE_LOG(LogSyInteraction, Warning, TEXT("Unknown interaction type tag: %s"), *InteractionType.ToString());
        CachedEntityComponent->BroadcastEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.End")));
    }
    
}

void USyInteractionComponent::HandleInteractionFinish() const
{
    CachedEntityComponent->BroadcastEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.End")));
}

void USyInteractionComponent::ProcessBasicInteraction(const FInstancedStruct& InteractionInfoStruct)
{
    const FSyBasicInteractInfo* BasicInfo = InteractionInfoStruct.GetPtr<FSyBasicInteractInfo>();
    if (!BasicInfo)
    {   
        UE_LOG(LogSyInteraction, Error, TEXT("Failed to get FSyBasicInteractInfo from InstancedStruct."));
        return;
    }

    if (OnBasicInteractionRequested.IsBound())
    {
        // 视情况加参数吧
        OnBasicInteractionRequested.Broadcast(this);
        // 记得在内部交互实现结束时给个 HandleInteractionFinish()
    }
    else
    {
        UE_LOG(LogSyInteraction, Log, TEXT("Basic interaction requested, but no listeners bound to OnBasicInteractionRequested."));
    }
    // CachedEntityComponent->BroadcastEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Basic")));
}

void USyInteractionComponent::ProcessFlowInteraction(const FInstancedStruct& InteractionInfoStruct)
{
    const FSyFlowInteractInfo* FlowInfo = InteractionInfoStruct.GetPtr<FSyFlowInteractInfo>();
    if (!FlowInfo)
    {
        UE_LOG(LogSyInteraction, Error, TEXT("Failed to get FSyFlowInteractInfo from InstancedStruct."));
        return;
    }

    if (FlowInfo->FlowAsset.IsNull())
    {
        UE_LOG(LogSyInteraction, Error, TEXT("Flow interaction requested, but FlowAsset is null."));
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogSyInteraction, Error, TEXT("Cannot process Flow interaction without an owner Actor."));
        return;
    }

    UFlowComponent* FlowComponent = Owner->FindComponentByClass<UFlowComponent>();
    if (!FlowComponent)
    {
        UE_LOG(LogSyInteraction, Log, TEXT("Actor %s: FlowComponent not found, adding dynamically."), *Owner->GetName());
        FlowComponent = NewObject<UFlowComponent>(Owner, UFlowComponent::StaticClass(), TEXT("AddedFlowComponent"));
        if (FlowComponent)
        {
            FlowComponent->RegisterComponent();
        }
        else
        {
             UE_LOG(LogSyInteraction, Error, TEXT("Failed to dynamically add FlowComponent to Actor %s."), *Owner->GetName());
             return;
        }
    }

    if (UFlowSubsystem* FlowSubsystem = Owner->GetWorld()->GetGameInstance()->GetSubsystem<UFlowSubsystem>())
    {
        UE_LOG(LogSyInteraction, Log, TEXT("Starting Flow Asset '%s' on Actor '%s' with component '%s'."), 
            *FlowInfo->FlowAsset.ToSoftObjectPath().ToString(), 
            *Owner->GetName(),
            *FlowComponent->GetName());
        
        // FlowSubsystem->StartFlowForComponent(FlowComponent, FlowInfo->FlowAsset.LoadSynchronous(), FlowInfo->InputPayload);
        FlowSubsystem->StartRootFlow(FlowComponent, FlowInfo->FlowAsset.LoadSynchronous(), TScriptInterface<IFlowDataPinValueSupplierInterface>());

        // TODO: 常用礼仪是挂个监听Flow完成，但好像不支持
        // 所以后续到 Flow 里实现一个节点，处理对话完成时，基于Owner尝试发送 Event.Interaction.End 吧（）
        // 最好是直接拓展一个 Flow 类，然后实现在Finish流程中

        // CachedEntityComponent->BroadcastEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.Flow")));
    }
    else
    {
        UE_LOG(LogSyInteraction, Error, TEXT("Failed to get FlowSubsystem."));
        CachedEntityComponent->BroadcastEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Interaction.End")));
    }
}

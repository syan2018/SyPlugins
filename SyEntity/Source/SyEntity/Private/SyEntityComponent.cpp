#include "SyEntityComponent.h"
#include "SyCore/Public/Identity/SyIdentityComponent.h"
#include "SyCore/Public/Messaging/SyMessageComponent.h"
#include "Registry/SyEntityRegistry.h"
#include "States/SyStateComponent.h"
#include "Manager/SyStateManagerSubsystem.h"
#include "Operations/OperationTypes.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"


USyEntityComponent::USyEntityComponent()
{
    // 设置组件属性
    PrimaryComponentTick.bCanEverTick = false;
    bIsInitialized = false;
    bRegistered = false;
    
    // 创建核心依赖组件 - IdentityComponent
    IdentityComponent = CreateDefaultSubobject<USyIdentityComponent>(TEXT("IdentityComponent"));
}

void USyEntityComponent::OnComponentCreated()
{
    Super::OnComponentCreated();
    
    // 在编辑器中创建组件时，尝试初始化
    if (!bIsInitialized)
    {
        InitializeEntity();
    }
}

void USyEntityComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // 如果尚未初始化，则调用初始化
    if (!bIsInitialized)
    {
        InitializeEntity();
    }
}

void USyEntityComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 注销实体
    if (bRegistered)
    {
        UnregisterFromRegistry();
    }
    
    Super::EndPlay(EndPlayReason);
}

void USyEntityComponent::InitializeEntity(bool bForceReinitialization)
{
    // 防止重复初始化，除非强制重新初始化
    if (bIsInitialized && !bForceReinitialization)
    {
        return;
    }
    
    // Phase 1: 创建和注册所有依赖组件
    EnsureDependentComponents();
    
    // Phase 2: 按依赖顺序初始化组件
    InitializeComponentsInOrder();
    
    // 注册到Registry
    RegisterWithRegistry();
    
    // 标记为已初始化
    bIsInitialized = true;
}

void USyEntityComponent::InitializeComponentsInOrder()
{
    // 1. 初始化 Identity 组件（无依赖）
    if (IdentityComponent)
    {
        IdentityComponent->GenerateEntityId();
        
        // 如果IdentityComponent已经有ID，则直接触发ID就绪事件
        if (IdentityComponent->GetEntityId().IsValid())
        {
            HandleEntityIdReady();
        }
    }
    
    // 2. 初始化 Message 组件（依赖 Identity）
    if (MessageComponent && IdentityComponent)
    {
        // MessageComponent 已经在 BeginPlay 中会自动获取 Identity
        // 这里可以添加额外的初始化逻辑（如果需要）
    }
    
    // 3. 初始化 State 组件（依赖 EntityComponent）
    if (StateComponent)
    {
        // StateComponent 在其自己的 BeginPlay 中会初始化
        // 由于我们确保了 EntityComponent 先初始化，时序已正确
    }
    
    // 4. 绑定所有组件的委托
    BindComponentDelegates();
}

void USyEntityComponent::EnsureDependentComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // 1. 确保MessageComponent存在
    if (!MessageComponent)
    {
        MessageComponent = Owner->FindComponentByClass<USyMessageComponent>();
        if (!MessageComponent)
        {
            MessageComponent = NewObject<USyMessageComponent>(Owner, USyMessageComponent::StaticClass());
            MessageComponent->RegisterComponent();
            ManagedSyComponents.Add(MessageComponent);
        }
    }

    // 2. 确保StateComponent存在
    if (!StateComponent)
    {
        StateComponent = Owner->FindComponentByClass<USyStateComponent>();
        if (!StateComponent)
        {
            StateComponent = NewObject<USyStateComponent>(Owner, USyStateComponent::StaticClass());
            StateComponent->RegisterComponent();
            ManagedSyComponents.Add(StateComponent);
        }
    }

    // 3. 收集其他Sy系列组件
    TArray<UActorComponent*> Components;
    Owner->GetComponents(Components);
    
    for (UActorComponent* Comp : Components)
    {
        // 使用接口判断是否是Sy系列组件，且不是已管理的必要组件
        if (Comp != this && 
            Comp != IdentityComponent && 
            Comp != MessageComponent && 
            Comp != StateComponent &&
            Comp->GetClass()->ImplementsInterface(USyComponentInterface::StaticClass()))
        {
            ManagedSyComponents.AddUnique(Comp);
        }
    }
}

void USyEntityComponent::RegisterWithRegistry()
{
    if (bRegistered || !IdentityComponent || !IdentityComponent->GetEntityId().IsValid())
    {
        return;
    }
    
    // 获取EntityRegistry子系统
    if (UWorld* World = GetWorld())
    {
        if (USyEntityRegistry* Registry = World->GetSubsystem<USyEntityRegistry>())
        {
            Registry->RegisterEntity(this);
            bRegistered = true;
        }
    }
}

void USyEntityComponent::UnregisterFromRegistry()
{
    if (!bRegistered)
    {
        return;
    }
    
    // 获取EntityRegistry子系统
    if (UWorld* World = GetWorld())
    {
        if (USyEntityRegistry* Registry = World->GetSubsystem<USyEntityRegistry>())
        {
            Registry->UnregisterEntity(this);
            bRegistered = false;
        }
    }
}

void USyEntityComponent::BindComponentDelegates()
{
    // 绑定 StateComponent 的本地数据变更事件
    if (StateComponent)
    {
        // 先移除旧的绑定（如果有的话），防止重复绑定
        StateComponent->OnEffectiveStateChanged.RemoveAll(this);
        // 添加新的绑定
        StateComponent->OnEffectiveStateChanged.AddUObject(this, &USyEntityComponent::HandleLocalStateDataChanged);
    }

    // TODO: 绑定其他组件（如 IdentityComponent）的事件
    // if (IdentityComponent)
    // {
    //     IdentityComponent->OnIdReadyDelegate.AddUObject(this, &USyEntityComponent::HandleEntityIdReady); // 假设有这样的委托
    // }
}

void USyEntityComponent::HandleLocalStateDataChanged()
{
    // 广播实体状态已更新事件
    OnEntityStateUpdated.Broadcast();
}

void USyEntityComponent::HandleEntityIdReady()
{
    // 广播ID就绪事件
    OnEntityIdReady.Broadcast();
    
    // 如果尚未注册，则注册到Registry
    if (!bRegistered)
    {
        RegisterWithRegistry();
    }
}

FGuid USyEntityComponent::GetEntityId() const
{
    if (IdentityComponent)
    {
        return IdentityComponent->GetEntityId();
    }
    
    return FGuid();
}

FGameplayTagContainer USyEntityComponent::GetEntityTags() const
{
    if (IdentityComponent)
    {
        return IdentityComponent->GetEntityTags();
    }
    
    return FGameplayTagContainer();
}

FName USyEntityComponent::GetEntityAlias() const
{
    if (IdentityComponent)
    {
        return IdentityComponent->GetEntityAlias();
    }
    
    return NAME_None;
}

// --- 4.4 语义化接口 - 区分消息和状态 ---

// 临时事件接口（通过 MessageBus）
bool USyEntityComponent::BroadcastEvent(const FGameplayTag& EventType)
{
    if (MessageComponent)
    {
        return MessageComponent->SendMessage(EventType);
    }
    
    return false;
}

bool USyEntityComponent::BroadcastEventWithMetadata(const FGameplayTag& EventType, const TMap<FName, FString>& Metadata)
{
    if (MessageComponent)
    {
        return MessageComponent->SendMessageWithMetadata(EventType, Metadata);
    }
    
    return false;
}

// 持久状态接口（通过 StateManager）
bool USyEntityComponent::ApplyStateOperation(const FSyOperation& Operation)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance)
    {
        return false;
    }
    
    USyStateManagerSubsystem* StateManager = GameInstance->GetSubsystem<USyStateManagerSubsystem>();
    if (!StateManager)
    {
        return false;
    }
    
    return StateManager->RecordOperation(Operation);
}

void USyEntityComponent::ApplyTemporaryStateModifications(const FSyStateParameterSet& TempModifications)
{
    if (StateComponent)
    {
        StateComponent->ApplyTemporaryModifications(TempModifications);
    }
} 
#include "SyEntityComponent.h"
#include "SyCore/Public/Identity/SyIdentityComponent.h"
#include "SyCore/Public/Messaging/SyMessageComponent.h"
#include "Registry/SyEntityRegistry.h"
#include "States/SyStateComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"


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
    
    
    EnsureDependentComponents();
    
    
    // 绑定组件委托
    BindComponentDelegates();
    
    // 处理 Identity
    if (IdentityComponent)
    {
        IdentityComponent->GenerateEntityId();
        
        // 如果IdentityComponent已经有ID，则直接触发ID就绪事件
        if (IdentityComponent->GetEntityId().IsValid())
        {
            HandleEntityIdReady();
        }
    }
    
    // 注册到Registry
    RegisterWithRegistry();
    
    // 标记为已初始化
    bIsInitialized = true;
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

bool USyEntityComponent::SendMessage(const FGameplayTag& MessageType)
{
    if (MessageComponent)
    {
        return MessageComponent->SendMessage(MessageType);
    }
    
    return false;
}

bool USyEntityComponent::SendMessageWithMetadata(const FGameplayTag& MessageType, const TMap<FName, FString>& Metadata)
{
    if (MessageComponent)
    {
        return MessageComponent->SendMessageWithMetadata(MessageType, Metadata);
    }
    
    return false;
} 
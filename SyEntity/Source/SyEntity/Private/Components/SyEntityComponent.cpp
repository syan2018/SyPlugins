#include "Components/SyEntityComponent.h"
#include "SyCore/Public/Identity/SyIdentityComponent.h"
#include "SyCore/Public/Messaging/SyMessageComponent.h"
#include "Components/SyStateComponent.h"
#include "Registry/SyEntityRegistry.h"
#include "States/SyStateManager.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"

// 模板实现必须在头文件中，这里只是占位
template<typename T>
T* USyEntityComponent::FindSyComponent() const
{
    for (UActorComponent* Comp : ManagedSyComponents)
    {
        if (T* TypedComp = Cast<T>(Comp))
        {
            return TypedComp;
        }
    }
    // 也可以直接检查已知的主要组件引用
    if (T* TypedComp = Cast<T>(StateComponent)) return TypedComp;
    if (T* TypedComp = Cast<T>(MessageComponent)) return TypedComp;
    if (T* TypedComp = Cast<T>(IdentityComponent)) return TypedComp;
    // ... 其他组件
    return nullptr;
}

USyEntityComponent::USyEntityComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bIsInitialized = false;
    bRegistered = false;
}

void USyEntityComponent::OnComponentCreated()
{
    Super::OnComponentCreated();
    
    // 在编辑器中创建组件时，尝试初始化
    if (GetWorld() && GetWorld()->IsEditorWorld())
    {
        InitializeEntity();
    }
}

void USyEntityComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // 如果尚未初始化，则进行初始化
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
    // 防止重复初始化
    if (bIsInitialized && !bForceReinitialization)
    {
        return;
    }
    
    // 确保依赖组件存在
    EnsureDependentComponents();
    
    // 创建并添加StateComponent（如果不存在）
    if (!StateComponent)
    {
        StateComponent = NewObject<USyStateComponent>(this);
        StateComponent->RegisterComponent();
        ManagedSyComponents.Add(StateComponent);
    }
    
    // 查找并收集Owner Actor上所有其他标记为Sy*的组件
    AActor* Owner = GetOwner();
    if (Owner)
    {
        // 使用GetComponentsByClass获取所有组件，然后转换为数组
        TArray<UActorComponent*> Components;
        Owner->GetComponents(Components);
        
        for (UActorComponent* Comp : Components)
        {
            // 检查组件名称是否以"Sy"开头
            if (Comp != this && Comp->GetName().StartsWith(TEXT("Sy")))
            {
                ManagedSyComponents.AddUnique(Comp);
            }
        }
    }
    
    // 绑定状态变更事件
    BindStateChangeDelegate();
    
    // 监听IdentityComponent的ID生成事件
    if (IdentityComponent)
    {
        // 注意：这里需要IdentityComponent暴露OnEntityIdGenerated事件
        // 如果IdentityComponent没有暴露此事件，需要修改SyIdentityComponent
        // 或者使用其他方式监听ID生成
        
        // 如果ID已经有效，直接触发事件
        if (IdentityComponent->HasValidId())
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
    
    // 确保IdentityComponent存在
    if (!IdentityComponent)
    {
        IdentityComponent = Owner->FindComponentByClass<USyIdentityComponent>();
        if (!IdentityComponent)
        {
            IdentityComponent = NewObject<USyIdentityComponent>(Owner);
            IdentityComponent->RegisterComponent();
        }
    }
    
    // 确保MessageComponent存在
    if (!MessageComponent)
    {
        MessageComponent = Owner->FindComponentByClass<USyMessageComponent>();
        if (!MessageComponent)
        {
            MessageComponent = NewObject<USyMessageComponent>(Owner);
            MessageComponent->RegisterComponent();
        }
    }
}

void USyEntityComponent::RegisterWithRegistry()
{
    if (!bRegistered && GetWorld())
    {
        // 获取EntityRegistry子系统
        USyEntityRegistry* Registry = GetWorld()->GetSubsystem<USyEntityRegistry>();
        if (Registry)
        {
            Registry->RegisterEntity(this);
            bRegistered = true;
        }
    }
}

void USyEntityComponent::UnregisterFromRegistry()
{
    if (bRegistered && GetWorld())
    {
        // 获取EntityRegistry子系统
        USyEntityRegistry* Registry = GetWorld()->GetSubsystem<USyEntityRegistry>();
        if (Registry)
        {
            Registry->UnregisterEntity(this);
            bRegistered = false;
        }
    }
}

void USyEntityComponent::BindStateChangeDelegate()
{
    if (StateComponent)
    {
        // 注意：这里需要StateComponent暴露InternalOnStateChanged事件
        // 如果StateComponent没有暴露此事件，需要修改SyStateComponent
        // 或者使用其他方式监听状态变更
        
        // 这里假设StateComponent有一个InternalOnStateChanged事件
        // StateComponent->InternalOnStateChanged.AddUObject(this, &USyEntityComponent::HandleInternalStateChange);
    }
}

void USyEntityComponent::HandleInternalStateChange(const FGameplayTag& StateTag, bool bNewValue)
{
    // 广播状态变更事件
    OnEntityStateChanged.Broadcast(StateTag, bNewValue);
}

void USyEntityComponent::HandleEntityIdReady()
{
    // 广播ID就绪事件
    OnEntityIdReady.Broadcast();
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

bool USyEntityComponent::GetState(const FGameplayTag& StateTag) const
{
    if (StateComponent)
    {
        return StateComponent->GetState(StateTag);
    }
    return false;
}

void USyEntityComponent::SetState(const FGameplayTag& StateTag, bool bNewValue, bool bSyncGlobal)
{
    if (StateComponent)
    {
        StateComponent->SetState(StateTag, bNewValue, true, bSyncGlobal);
    }
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
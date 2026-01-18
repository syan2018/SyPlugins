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
    
    // 不在编辑器创建时初始化，避免重复创建组件
    // 初始化统一在 BeginPlay 中处理
}

void USyEntityComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // 延迟初始化到下一帧，确保所有组件的 BeginPlay 都已完成
    // 这样可以避免组件 BeginPlay 调用顺序不确定导致的问题
    if (!bIsInitialized)
    {
        // 使用 Timer 延迟一帧
        if (UWorld* World = GetWorld())
        {
            FTimerHandle TimerHandle;
            World->GetTimerManager().SetTimerForNextTick([this]()
            {
                if (this && IsValid(this) && !bIsInitialized)
                {
                    InitializeEntity();
                }
            });
        }
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
    // ✅ 统一初始化流程：所有组件都通过 OnSyComponentInitialized() 初始化
    
    // 1. 初始化 Identity 组件（无依赖）
    if (IdentityComponent)
    {
        IdentityComponent->GenerateEntityId();
        
        if (IdentityComponent->GetEntityId().IsValid())
        {
            HandleEntityIdReady();
        }
    }
    
    // 2. 绑定组件委托（为后续状态广播做准备）
    BindComponentDelegates();
    
    // 3. ✅ 按阶段顺序初始化所有 Sy 组件
    //    Phase::Core (0)        - StateComponent 初始化数据并广播初始状态
    //    Phase::Functional (100) - Interaction/Spawn 组件处理初始状态
    InitializeSyComponentsByPhase();
}

void USyEntityComponent::EnsureDependentComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // 1. 查找 MessageComponent（优先使用已存在的）
    if (!MessageComponent)
    {
        MessageComponent = Owner->FindComponentByClass<USyMessageComponent>();
    }
    
    // 如果没有找到，只在运行时动态创建
    if (!MessageComponent && Owner->GetWorld() && Owner->GetWorld()->IsGameWorld())
    {
        MessageComponent = NewObject<USyMessageComponent>(Owner, USyMessageComponent::StaticClass(), TEXT("SyMessageComponent"));
        MessageComponent->RegisterComponent();
        ManagedSyComponents.Add(MessageComponent);
    }

    // 2. 查找 StateComponent（必须在蓝图中已添加）
    if (!StateComponent)
    {
        // 使用 GetComponents 遍历查找，确保能找到所有实例
        TArray<USyStateComponent*> StateComponents;
        Owner->GetComponents<USyStateComponent>(StateComponents);
        
        if (StateComponents.Num() > 0)
        {
            // 如果有多个，发出警告
            if (StateComponents.Num() > 1)
            {
                UE_LOG(LogTemp, Error, TEXT("❌ Actor %s has %d StateComponents! This will cause problems!"), 
                    *GetNameSafe(Owner), StateComponents.Num());
            }
            
            // 使用第一个找到的
            StateComponent = StateComponents[0];
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ No StateComponent found on Actor %s! Please add SyStateComponent in Blueprint."), 
                *GetNameSafe(Owner));
        }
    }
    
    // 不再动态创建 StateComponent！必须在蓝图中手动添加
    if (!StateComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ CRITICAL: Actor %s has no StateComponent!"), 
            *GetNameSafe(Owner));
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
        // 添加新的绑定 - EntityComponent 也需要监听状态变化
        StateComponent->OnEffectiveStateChanged.AddUObject(this, &USyEntityComponent::HandleLocalStateDataChanged);
    }
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

void USyEntityComponent::InitializeSyComponentsByPhase()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // 收集所有实现了 ISyComponentInterface 的组件
    struct FSyComponentWithPhase
    {
        ISyComponentInterface* Component;
        ESyComponentInitPhase Phase;
        
        bool operator<(const FSyComponentWithPhase& Other) const
        {
            // 按阶段排序，阶段值小的先初始化
            return static_cast<uint8>(Phase) < static_cast<uint8>(Other.Phase);
        }
    };
    
    TArray<FSyComponentWithPhase> SyComponents;
    
    TArray<UActorComponent*> AllComponents;
    Owner->GetComponents(AllComponents);
    
    for (UActorComponent* Comp : AllComponents)
    {
        if (Comp && Comp != this && Comp->GetClass()->ImplementsInterface(USyComponentInterface::StaticClass()))
        {
            if (ISyComponentInterface* SyComp = Cast<ISyComponentInterface>(Comp))
            {
                FSyComponentWithPhase Item;
                Item.Component = SyComp;
                Item.Phase = SyComp->GetInitializationPhase();
                SyComponents.Add(Item);
            }
        }
    }
    
    // 按阶段排序
    SyComponents.Sort();
    
    // 按顺序初始化
    for (const FSyComponentWithPhase& Item : SyComponents)
    {
        Item.Component->OnSyComponentInitialized();
    }
}

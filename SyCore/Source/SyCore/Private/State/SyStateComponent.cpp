// Copyright Epic Games, Inc. All Rights Reserved.

#include "State/SyStateComponent.h"
#include "Entity/SyEntityComponent.h" // Include Entity Component
#include "Entity/SyEntityRegistry.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"
#include "State/Types/StateContainerTypes.h" // Included via header, but good practice
#include "State/Types/StateParameterTypes.h" // Included via header, but good practice

DEFINE_LOG_CATEGORY_STATIC(LogSyStateComponent, Log, All); // 添加日志分类

USyStateComponent::USyStateComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bIsFullyInitialized = false;
}

void USyStateComponent::BeginPlay()
{
    Super::BeginPlay();

    // 检查是否有重复的 StateComponent
    if (AActor* Owner = GetOwner())
    {
        TArray<USyStateComponent*> AllStateComps;
        Owner->GetComponents<USyStateComponent>(AllStateComps);
        if (AllStateComps.Num() > 1)
        {
            UE_LOG(LogSyStateComponent, Error, TEXT("❌ Actor %s has %d StateComponents! Please check Construction Script."), 
                *GetNameSafe(Owner), AllStateComps.Num());
        }
    }

    // BeginPlay 不做任何初始化，只做基础检查
    // 所有初始化逻辑都在 OnSyComponentInitialized() 中完成
}

void USyStateComponent::OnSyComponentInitialized()
{
    // ✅ 统一初始化入口：所有初始化逻辑都在这里完成
    
    // 1. 查找并缓存 EntityComponent
    FindAndCacheEntityComponent();

    // 2. 应用 Profile 初始化数据（若有）
    if (StateProfile)
    {
        UE_LOG(LogSyStateComponent, Log, TEXT("%s: Applying profile initialization data to Default layer."), *GetNameSafe(GetOwner()));
        LayeredState.ApplyParameterSetToLayer(ESyStateLayer::Default, StateProfile->DefaultInitData);
    }

    // 3. 应用组件默认初始化数据（覆盖/补充）
    UE_LOG(LogSyStateComponent, Log, TEXT("%s: Applying initialization data to Default layer."), *GetNameSafe(GetOwner()));
    LayeredState.ApplyParameterSetToLayer(ESyStateLayer::Default, DefaultInitData);

    // 4. 初始化后端
    InitializeBackends();
    
    // 5. 标记为已完全初始化
    bIsFullyInitialized = true;
    
    // 6. ✅ 广播初始状态（此时所有 Core 阶段组件都已准备好）
    UE_LOG(LogSyStateComponent, Log, TEXT("%s: StateComponent fully initialized, broadcasting initial state."), *GetNameSafe(GetOwner()));
    OnEffectiveStateChanged.Broadcast();
}

void USyStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void USyStateComponent::FindAndCacheEntityComponent()
{
    if (!GetOwner())
    {
        return;
    }

    // 查找EntityComponent
    EntityComponent = GetOwner()->FindComponentByClass<USyEntityComponent>();
    if (!EntityComponent)
    {
        UE_LOG(LogSyStateComponent, Warning, TEXT("%s: Could not find EntityComponent on owner actor."), *GetNameSafe(GetOwner()));
    }
}

FGameplayTag USyStateComponent::GetTargetTypeTag() const
{
    if (!EntityComponent)
    {
        return FGameplayTag();
    }

    // 获取EntityComponent的所有Tags
    FGameplayTagContainer EntityTags = EntityComponent->GetEntityTags();
    
    // 返回第一个Tag作为目标类型标签
    if (EntityTags.Num() > 0)
    {
        return EntityTags.First();
    }

    return FGameplayTag();
}

void USyStateComponent::ApplyInitializationData(const FSyStateParameterSet& InitData)
{
    UE_LOG(LogSyStateComponent, Log, TEXT("%s: Applying initialization data to Default layer."), *GetNameSafe(GetOwner()));
    
    // 应用到默认层
    LayeredState.ApplyParameterSetToLayer(ESyStateLayer::Default, InitData);

    // Broadcast that the effective state has changed (只有在完全初始化后才广播)
    if (bIsFullyInitialized)
    {
        OnEffectiveStateChanged.Broadcast();
    }
}

void USyStateComponent::ApplyTemporaryModifications(const FSyStateParameterSet& TempModifications)
{
    UE_LOG(LogSyStateComponent, Log, TEXT("%s: Applying temporary modifications to Temporary layer."), *GetNameSafe(GetOwner()));
    
    // 应用到临时层
    LayeredState.ApplyParameterSetToLayer(ESyStateLayer::Temporary, TempModifications);

    // Broadcast that the effective state has changed
    OnEffectiveStateChanged.Broadcast();
}

void USyStateComponent::ClearStateLayer(ESyStateLayer Layer)
{
    UE_LOG(LogSyStateComponent, Log, TEXT("%s: Clearing layer %d."), *GetNameSafe(GetOwner()), (int32)Layer);
    
    LayeredState.ClearLayer(Layer);

    // Broadcast that the effective state has changed
    OnEffectiveStateChanged.Broadcast();
}

// --- State Access ---

const FSyStateCategories& USyStateComponent::GetStateLayer(ESyStateLayer Layer) const
{
    return LayeredState.GetLayer(Layer);
}

FSyStateCategories USyStateComponent::GetEffectiveStateCategories() const
{
    // 使用分层容器的缓存机制获取有效状态
    return LayeredState.GetEffectiveState();
}

bool USyStateComponent::GetEffectiveStateParam(FGameplayTag StateTag, FInstancedStruct& OutParam) const
{
    // 获取有效状态（已自动按优先级合并）
    FSyStateCategories EffectiveState = GetEffectiveStateCategories();
    
    if (const FSyStateMetadatas* Metadatas = EffectiveState.GetStateDataMap().Find(StateTag))
    {
        // Find the first valid metadata param
        for(const auto& MetaPtr : Metadatas->MetadataArray)
        {
            if(const USyStateMetadataBase* Metadata = Cast<USyStateMetadataBase>(MetaPtr))
            {
                 OutParam = Metadata->GetValueStruct();
                 if (OutParam.IsValid()) return true;
            }
        }
    }

    // Not found
    OutParam.Reset();
    return false;
}

bool USyStateComponent::ApplyStateChange(const FSyStateChangeRequest& Request)
{
    if (!GetWorld())
    {
        return false;
    }

    if (!bBackendsInitialized)
    {
        InitializeBackends();
    }

    // 允许跨实体转发（唯一入口）
    if (Request.TargetEntityId.IsValid() && EntityComponent && Request.TargetEntityId != EntityComponent->GetEntityId())
    {
        if (USyEntityRegistry* Registry = GetWorld()->GetSubsystem<USyEntityRegistry>())
        {
            if (USyEntityComponent* TargetEntity = Registry->GetEntityById(Request.TargetEntityId))
            {
                if (USyStateComponent* TargetState = TargetEntity->FindSyComponent<USyStateComponent>())
                {
                    return TargetState->ApplyStateChange(Request);
                }
            }
        }
        UE_LOG(LogSyStateComponent, Warning, TEXT("ApplyStateChange: TargetEntityId not found or missing USyStateComponent. TargetEntityId=%s"), *Request.TargetEntityId.ToString());
        return false;
    }

    bool bApplied = ApplyViaBackends(Request);
    if (bApplied)
    {
        OnEffectiveStateChanged.Broadcast();
        return true;
    }

    UE_LOG(LogSyStateComponent, Error, TEXT("ApplyStateChange failed: no backend handled the request. StateTag=%s Scope=%d Layer=%d"),
        *Request.StateTag.ToString(), (int32)Request.Scope, (int32)Request.Layer);
    return false;
}

bool USyStateComponent::TryGetStateValueStruct(FGameplayTag StateTag, FInstancedStruct& OutValue) const
{
    if (!bBackendsInitialized)
    {
        const_cast<USyStateComponent*>(this)->InitializeBackends();
    }

    return TryGetViaBackends(StateTag, OutValue);
}

void USyStateComponent::InitializeBackends()
{
    if (bBackendsInitialized)
    {
        return;
    }

    BuildBackendInstances();

    for (USyStateBackendBase* Backend : Backends)
    {
        if (Backend)
        {
            Backend->InitializeBackend(this);
        }
    }

    SortBackends();
    bBackendsInitialized = true;

    if (Backends.Num() == 0)
    {
        UE_LOG(LogSyStateComponent, Warning, TEXT("%s: No state backends configured. State changes will be rejected."), *GetNameSafe(GetOwner()));
    }
}

void USyStateComponent::BuildBackendInstances()
{
    if (Backends.Num() > 0)
    {
        return;
    }

    TSet<UClass*> UniqueTypes;

    if (StateProfile)
    {
        for (const TObjectPtr<USyStateBackendBase>& Template : StateProfile->BackendInstances)
        {
            if (!Template)
            {
                continue;
            }

            USyStateBackendBase* NewBackend = DuplicateObject<USyStateBackendBase>(Template, this);
            if (NewBackend)
            {
                Backends.Add(NewBackend);
                UniqueTypes.Add(NewBackend->GetClass());
            }
        }
    }

    TArray<TSubclassOf<USyStateBackendBase>> TypesToCreate;
    if (StateProfile)
    {
        TypesToCreate.Append(StateProfile->BackendTypes);
    }
    TypesToCreate.Append(BackendTypes);

    for (const TSubclassOf<USyStateBackendBase>& Type : TypesToCreate)
    {
        if (!Type)
        {
            continue;
        }

        if (UniqueTypes.Contains(*Type))
        {
            continue;
        }

        UniqueTypes.Add(*Type);
        USyStateBackendBase* NewBackend = NewObject<USyStateBackendBase>(this, Type);
        if (NewBackend)
        {
            Backends.Add(NewBackend);
        }
    }
}

void USyStateComponent::SortBackends()
{
	Backends.RemoveAll([](const TObjectPtr<USyStateBackendBase>& Item)
	{
		return !Item;
	});

	Backends.Sort([](const USyStateBackendBase& A, const USyStateBackendBase& B)
	{
		return A.GetBackendPriority() > B.GetBackendPriority();
	});
}

bool USyStateComponent::ApplyViaBackends(const FSyStateChangeRequest& LocalRequest)
{
    for (USyStateBackendBase* Backend : Backends)
    {
        if (!Backend)
        {
            continue;
        }

        if (!Backend->CanHandleChange(LocalRequest))
        {
            continue;
        }

        if (Backend->ApplyChange(LocalRequest))
        {
            return true;
        }
    }

    return false;
}

bool USyStateComponent::TryGetViaBackends(const FGameplayTag& StateTag, FInstancedStruct& OutValue) const
{
    for (USyStateBackendBase* Backend : Backends)
    {
        if (!Backend)
        {
            continue;
        }

        if (Backend->TryGetValueStruct(StateTag, OutValue))
        {
            return true;
        }
    }

    OutValue.Reset();
    return false;
}

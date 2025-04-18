#include "Components/SySpawnComponent.h"
#include "States/SyStateComponent.h"
#include "GameFramework/Actor.h"
#include "Logging/LogMacros.h"
#include "Metadatas/BasicMetadataValueTypes.h"
#include "SyStateCore/Public/StateMetadataTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogSySpawn, Log, All);

USySpawnComponent::USySpawnComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void USySpawnComponent::BeginPlay()
{
    Super::BeginPlay();

    // 查找并缓存StateComponent
    FindAndCacheStateComponent();

    // 如果找到StateComponent，绑定状态变化事件
    if (StateComponent)
    {
        StateComponent->OnEffectiveStateChanged.AddUObject(this, &USySpawnComponent::HandleStateChanged);
        // 初始状态检查
        HandleStateChanged();
    }
    else
    {
        UE_LOG(LogSySpawn, Warning, TEXT("%s: Could not find StateComponent on owner actor."), *GetNameSafe(GetOwner()));
    }
}

void USySpawnComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 解绑状态变化事件
    if (StateComponent)
    {
        StateComponent->OnEffectiveStateChanged.RemoveAll(this);
    }

    Super::EndPlay(EndPlayReason);
}

void USySpawnComponent::FindAndCacheStateComponent()
{
    if (!GetOwner())
    {
        return;
    }

    // 查找StateComponent
    StateComponent = GetOwner()->FindComponentByClass<USyStateComponent>();
    if (!StateComponent)
    {
        UE_LOG(LogSySpawn, Warning, TEXT("%s: Could not find StateComponent on owner actor."), *GetNameSafe(GetOwner()));
    }
}

void USySpawnComponent::HandleStateChanged()
{
    if (!StateComponent)
    {
        return;
    }

    // 检查"State.Spawner.Enable"标签的状态
    const FGameplayTag SpawnerEnableTag = FGameplayTag::RequestGameplayTag(TEXT("State.Spawner.Enable"));
    const FSyStateCategories& CurrentState = StateComponent->GetEffectiveStateCategories();
    
    // 查找Spawner.Enable标签的第一个元数据
    if (USyStateMetadataBase* Metadata = CurrentState.FindFirstStateMetadata<USyStateMetadataBase>(SpawnerEnableTag))
    {
        // 尝试获取布尔值
        FSyBoolValue bIsSpawnerEnabled = false;
        
        if (Metadata->TryGetValue(bIsSpawnerEnabled))
        {
            // 根据状态更新生成能力
            if (bIsSpawnerEnabled.Value)
            {
                // 如果生成器被启用，且当前没有生成的实体，则尝试生成
                if (!SpawnedActor.IsValid())
                {
                    // 使用默认的生成参数
                    FQuestSpawnParams DefaultParams;
                    DefaultParams.ActorScale = 1.0f;
                    DefaultParams.bNoCollisionFail = true;
                    
                    if (Spawn(DefaultParams))
                    {
                        UE_LOG(LogSySpawn, Verbose, TEXT("%s: Spawned actor due to State.Spawner.Enable tag."), *GetNameSafe(GetOwner()));
                    }
                }
            }
            else
            {
                // 如果生成器被禁用，且当前有生成的实体，则销毁
                if (SpawnedActor.IsValid())
                {
                    Despawn();
                    UE_LOG(LogSySpawn, Verbose, TEXT("%s: Despawned actor due to State.Spawner.Enable tag."), *GetNameSafe(GetOwner()));
                }
            }
        }
        else
        {
            UE_LOG(LogSySpawn, Warning, TEXT("%s: State.Spawner.Enable metadata is not a boolean value."), *GetNameSafe(GetOwner()));
        }
    }
    else
    {
        // 如果没有找到Spawner.Enable标签的元数据，默认不生成
        if (SpawnedActor.IsValid())
        {
            Despawn();
            UE_LOG(LogSySpawn, Verbose, TEXT("%s: Despawned actor due to missing State.Spawner.Enable tag."), *GetNameSafe(GetOwner()));
        }
    }
} 
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "SyCore/Public/Components/ISyComponentInterface.h"
#include "SyEntityComponent.generated.h"

// 前向声明
class USyIdentityComponent;
class USyMessageComponent;
class USyStateComponent;

/**
 * SyEntityComponent - 实体核心组件
 * 职责：
 * 1. 核心协调器，管理其他Sy*功能组件
 * 2. 提供统一的实体访问点
 * 3. 管理组件生命周期和初始化
 * 4. 实现组件间通信机制
 */
UCLASS(Blueprintable, ClassGroup=(SyEntity), meta=(BlueprintSpawnableComponent))
class SYENTITY_API USyEntityComponent : public UActorComponent, public ISyComponentInterface
{
    GENERATED_BODY()

public:
    USyEntityComponent();

    // 实现ISyComponentInterface接口
    virtual FName GetComponentType() const override { return TEXT("Entity"); }

    // --- 生命周期与初始化 ---
protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void OnComponentCreated() override; // 用于编辑器中或动态创建时

public:
    /**
     * @brief 在Actor的构造函数或BeginPlay中调用，确保所有依赖组件被创建和初始化。
     * @param bForceReinitialization 是否强制重新初始化，即使已经初始化过。
     */
    UFUNCTION(BlueprintCallable, Category = "SyEntity|Initialization")
    void InitializeEntity(bool bForceReinitialization = false);

    /**
     * @brief 检查实体是否已成功初始化。
     */
    UFUNCTION(BlueprintPure, Category = "SyEntity|Initialization")
    bool IsEntityInitialized() const { return bIsInitialized; }

    // --- 组件管理 ---
public:
    /**
     * @brief 获取指定类型的Sy功能组件。
     * @tparam T 组件类型 (例如 USyStateComponent)
     * @return 如果找到则返回组件指针，否则返回nullptr。
     */
    template<typename T>
    T* FindSyComponent() const;

    /**
     * @brief 获取实体的唯一ID。
     * @return 实体GUID，如果未初始化则无效。
     */
    UFUNCTION(BlueprintPure, Category = "SyEntity|Identity")
    FGuid GetEntityId() const;

    /**
     * @brief 获取实体的Gameplay Tag。
     */
    UFUNCTION(BlueprintPure, Category = "SyEntity|Identity")
    FGameplayTagContainer GetEntityTags() const;

    /**
     * @brief 获取实体的别名。
     */
    UFUNCTION(BlueprintPure, Category = "SyEntity|Identity")
    FName GetEntityAlias() const;

    /**
     * @brief 获取状态组件。
     */
    UFUNCTION(BlueprintPure, Category = "SyEntity|Components")
    USyStateComponent* GetStateComponent() const { return StateComponent; }

    /**
     * @brief 获取消息组件。
     */
    UFUNCTION(BlueprintPure, Category = "SyEntity|Components")
    USyMessageComponent* GetMessageComponent() const { return MessageComponent; }

    // --- 状态接口 (代理StateComponent) ---
    // 移除了 GetState/SetState 代理，建议直接访问 StateComponent->GetCurrentStateCategories()

    // --- 消息接口 (代理MessageComponent) - 用于临时事件 ---
public:
    /**
     * @brief 广播临时事件（通过 MessageBus）
     * @note 用于瞬时性、不可恢复的事件通知（如UI交互、音效触发）
     */
    UFUNCTION(BlueprintCallable, Category = "SyEntity|Events", meta=(DisplayName="Broadcast Event"))
    bool BroadcastEvent(const FGameplayTag& EventType);

    /**
     * @brief 广播带元数据的临时事件
     */
    UFUNCTION(BlueprintCallable, Category = "SyEntity|Events")
    bool BroadcastEventWithMetadata(const FGameplayTag& EventType, const TMap<FName, FString>& Metadata);
    
    // --- 状态操作接口 (通过 StateManager) - 用于持久状态 ---
    /**
     * @brief 记录状态操作（通过 StateManager）
     * @note 用于持久性、可恢复的状态变更（如角色属性、任务进度）
     * @param Operation 要记录的状态操作
     * @return 如果成功记录返回 true
     */
    UFUNCTION(BlueprintCallable, Category = "SyEntity|States", meta=(DisplayName="Apply State Operation"))
    bool ApplyStateOperation(const FSyOperation& Operation);
    
    /**
     * @brief 应用临时状态修改到 Temporary 层（如 Buff/Debuff）
     * @note 这些状态不会记录到 StateManager，仅影响本地实体
     * @param TempModifications 临时修改的参数集
     */
    UFUNCTION(BlueprintCallable, Category = "SyEntity|States")
    void ApplyTemporaryStateModifications(const FSyStateParameterSet& TempModifications);

    // --- 组件间通信 (核心机制) ---
public:
    /** 当实体状态数据发生更新时广播 (由 StateComponent 内部事件触发) */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEntityStateUpdated);
    UPROPERTY(BlueprintAssignable, Category = "SyEntity|Events")
    FOnEntityStateUpdated OnEntityStateUpdated;

    // 当实体ID生成或有效时广播 (由IdentityComponent触发)
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEntityIdReady);
    UPROPERTY(BlueprintAssignable, Category = "SyEntity|Events")
    FOnEntityIdReady OnEntityIdReady;

private:
    // --- 内部属性 ---
    // 核心依赖组件 - 在构造函数中创建
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SyEntity|Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USyIdentityComponent> IdentityComponent;

    // 可选组件 - 在初始化时创建
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SyEntity|Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USyMessageComponent> MessageComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SyEntity|Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USyStateComponent> StateComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SyEntity|Components", meta = (AllowPrivateAccess = "true"))
    TArray<TObjectPtr<UActorComponent>> ManagedSyComponents; // 存储所有Sy*组件的引用，便于统一管理

    bool bIsInitialized = false;
    bool bRegistered = false; // 是否已注册到Registry

    // --- 内部方法 - 两阶段初始化 ---
    /** Phase 1: 创建和注册所有依赖组件 */
    void EnsureDependentComponents();
    
    /** Phase 2: 按依赖顺序初始化组件 */
    void InitializeComponentsInOrder();
    
    /** 其他内部方法 */
    void RegisterWithRegistry();
    void UnregisterFromRegistry();
    void BindComponentDelegates(); // 绑定所有组件的委托
    
    void HandleLocalStateDataChanged(); // StateComponent 内部事件处理
    void HandleEntityIdReady(); // IdentityComponent ID生成事件处理
    
    /** 按阶段顺序初始化所有 Sy 组件 */
    void InitializeSyComponentsByPhase();
};

// Template implementation for FindSyComponent
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
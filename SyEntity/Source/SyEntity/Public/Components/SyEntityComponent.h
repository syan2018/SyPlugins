#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
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
class SYENTITY_API USyEntityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USyEntityComponent();

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

    // --- 消息接口 (代理MessageComponent) ---
public:
    /**
     * @brief 发送实体消息。
     */
    UFUNCTION(BlueprintCallable, Category = "SyEntity|Message")
    bool SendMessage(const FGameplayTag& MessageType);

    /**
     * @brief 发送带元数据的实体消息。
     */
    UFUNCTION(BlueprintCallable, Category = "SyEntity|Message")
    bool SendMessageWithMetadata(const FGameplayTag& MessageType, const TMap<FName, FString>& Metadata);

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

    UPROPERTY()
    TArray<TObjectPtr<UActorComponent>> ManagedSyComponents; // 存储所有Sy*组件的引用，便于统一管理

    bool bIsInitialized = false;
    bool bRegistered = false; // 是否已注册到Registry

    // --- 内部方法 ---
    void EnsureDependentComponents(); // 确保依赖组件存在
    void RegisterWithRegistry();
    void UnregisterFromRegistry();
    void BindComponentDelegates(); // 重命名，绑定所有组件的委托
    void HandleLocalStateDataChanged(); // StateComponent 内部事件处理
    void HandleEntityIdReady(); // IdentityComponent ID生成事件处理
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
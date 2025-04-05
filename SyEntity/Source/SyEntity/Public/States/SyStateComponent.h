#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "SyStateComponent.generated.h"

// 前向声明
class USyStateManager;

/**
 * SyStateComponent - 实体状态组件
 * 职责：
 * 1. 维护实体的本地状态集合
 * 2. 提供状态查询和修改接口
 * 3. 与全局StateManager同步状态
 * 4. 触发内部状态变更事件
 */
UCLASS(ClassGroup=(SyEntity), meta=(BlueprintSpawnableComponent))
class SYENTITY_API USyStateComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USyStateComponent();

    // --- 状态操作 ---
    /**
     * @brief 获取状态值。
     * @param StateTag 要查询的状态标签。
     * @param bDefaultValue 如果状态不存在时的默认返回值。
     * @return 状态的布尔值。
     */
    UFUNCTION(BlueprintPure, Category = "SyState")
    bool GetState(const FGameplayTag& StateTag, bool bDefaultValue = false) const;

    /**
     * @brief 设置状态值。
     * @param StateTag 要设置的状态标签。
     * @param bNewValue 新的状态值。
     * @param bSyncGlobal 是否尝试与StateManager同步。
     */
    UFUNCTION(BlueprintCallable, Category = "SyState")
    void SetState(const FGameplayTag& StateTag, bool bNewValue, bool bSyncGlobal = true);

    /**
     * @brief 检查是否存在某个状态。
     */
    UFUNCTION(BlueprintPure, Category = "SyState")
    bool HasState(const FGameplayTag& StateTag) const;

    // --- 全局同步 ---
    /**
     * @brief 从StateManager拉取状态覆盖本地状态。
     */
    UFUNCTION(BlueprintCallable, Category = "SyState|Sync")
    void PullStateFromManager();

    /**
     * @brief 将本地所有状态推送到StateManager。
     */
    UFUNCTION(BlueprintCallable, Category = "SyState|Sync")
    void PushStateToManager();

    // --- 配置 ---
    /**
     * @brief 初始状态配置 (可以在蓝图或数据资产中定义)。
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyState|Config")
    TMap<FGameplayTag, bool> InitialStates;

    /**
     * @brief 是否默认启用与全局StateManager的同步。
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyState|Config")
    bool bEnableGlobalSyncByDefault = true;

    // --- 内部事件 --- (供EntityComponent绑定)
    DECLARE_MULTICAST_DELEGATE_TwoParams(FInternalOnStateChanged, const FGameplayTag& /*StateTag*/, bool /*bNewValue*/);
    FInternalOnStateChanged InternalOnStateChanged;

protected:
    virtual void BeginPlay() override;
    virtual void InitializeComponent() override; // 更早的初始化点

private:
    UPROPERTY(VisibleAnywhere, Category = "SyState|Data", meta = (AllowPrivateAccess = "true"))
    TMap<FGameplayTag, bool> CurrentStates; // 存储当前状态

    UPROPERTY() // 缓存EntityComponent引用
    TObjectPtr<class USyEntityComponent> OwnerEntityComponent;

    UPROPERTY() // 缓存StateManager引用
    TObjectPtr<USyStateManager> StateManager;

    FGuid GetOwnerEntityId() const; // 辅助函数获取Owner ID
}; 
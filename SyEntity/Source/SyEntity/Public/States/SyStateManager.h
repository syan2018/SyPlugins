#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "SyStateManager.generated.h"

/**
 * 实体状态数据结构体，用于包装实体的状态Map
 */
USTRUCT(BlueprintType)
struct FSyEntityStateData
{
    GENERATED_BODY()

    // 状态Map：状态Tag -> 状态值
    UPROPERTY()
    TMap<FGameplayTag, bool> States;
};

/**
 * SyStateManager - 全局状态管理器
 * 职责：
 * 1. 全局管理实体状态
 * 2. 提供状态查询和更新接口
 * 3. 处理状态持久化
 * 4. 广播全局状态变更事件
 */
UCLASS()
class SYENTITY_API USyStateManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // --- 状态管理 ---
    /**
     * @brief 更新指定实体的特定状态。如果实体或状态不存在，则会创建。
     */
    UFUNCTION(BlueprintCallable, Category = "SyStateManager")
    void UpdateEntityState(const FGuid& EntityId, const FGameplayTag& StateTag, bool bNewValue);

    /**
     * @brief 获取指定实体的特定状态。
     * @param bFound 是否找到了实体和状态。
     * @return 状态值，如果未找到则为false。
     */
    UFUNCTION(BlueprintPure, Category = "SyStateManager")
    bool GetEntityState(const FGuid& EntityId, const FGameplayTag& StateTag, bool& bFound) const;

    /**
     * @brief 获取指定实体的所有状态。
     * @param bFound 是否找到了实体。
     * @return 包含实体所有状态的Map，如果未找到则为空。
     */
    UFUNCTION(BlueprintPure, Category = "SyStateManager")
    TMap<FGameplayTag, bool> GetAllEntityStates(const FGuid& EntityId, bool& bFound) const;

    /**
     * @brief 注册一个实体（通常不需要手动调用，StateComponent内部处理）。
     */
    UFUNCTION(BlueprintCallable, Category = "SyStateManager")
    void RegisterEntity(const FGuid& EntityId);

    /**
     * @brief 注销一个实体（通常不需要手动调用）。
     */
    UFUNCTION(BlueprintCallable, Category = "SyStateManager")
    void UnregisterEntity(const FGuid& EntityId);

    // --- 事件广播 (可选) ---
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGlobalStateChanged, const FGuid&, EntityId, const FGameplayTag&, StateTag, bool, bNewValue);
    UPROPERTY(BlueprintAssignable, Category = "SyStateManager|Events")
    FOnGlobalStateChanged OnGlobalStateChanged;

private:
    // 存储结构：实体ID -> 实体状态数据
    UPROPERTY()
    TMap<FGuid, FSyEntityStateData> GlobalEntityStates;
}; 
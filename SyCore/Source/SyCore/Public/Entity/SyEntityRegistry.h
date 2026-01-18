#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "SyEntityRegistry.generated.h"

// 前向声明
class USyEntityComponent;

/**
 * 实体标签索引结构体，用于包装标签对应的实体列表
 */
USTRUCT()
struct FSyEntityTagIndex
{
    GENERATED_BODY()

    // 标签对应的实体列表
    UPROPERTY()
    TArray<TObjectPtr<USyEntityComponent>> Entities;
};

/**
 * SyEntityRegistry - 实体注册表
 * 职责：
 * 1. 维护当前活跃实体的索引
 * 2. 提供通过ID、类型Tag等方式查询实体的接口
 */
UCLASS()
class SYCORE_API USyEntityRegistry : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    /**
     * @brief 注册实体。由EntityComponent在初始化时调用。
     */
    void RegisterEntity(USyEntityComponent* EntityComponent);

    /**
     * @brief 注销实体。由EntityComponent在销毁时调用。
     */
    void UnregisterEntity(USyEntityComponent* EntityComponent);

    /**
     * @brief 通过ID查找实体组件。
     */
    UFUNCTION(BlueprintPure, Category = "SyEntityRegistry")
    USyEntityComponent* GetEntityById(const FGuid& EntityId) const;

    /**
     * @brief 通过类型Tag查找所有匹配的实体组件。
     */
    UFUNCTION(BlueprintPure, Category = "SyEntityRegistry")
    TArray<USyEntityComponent*> GetEntitiesByTag(const FGameplayTag& EntityTag) const;

    /**
     * @brief 通过类型Tag查找所有匹配的实体组件（精确匹配）。
     */
    UFUNCTION(BlueprintPure, Category = "SyEntityRegistry")
    TArray<USyEntityComponent*> GetEntitiesByTagExact(const FGameplayTag& EntityTag) const;

    /**
     * @brief 获取所有已注册的实体组件。
     */
    UFUNCTION(BlueprintPure, Category = "SyEntityRegistry")
    TArray<USyEntityComponent*> GetAllRegisteredEntities() const;

private:
    // 索引结构
    UPROPERTY()
    TMap<FGuid, TObjectPtr<USyEntityComponent>> EntityIdMap;

    UPROPERTY()
    TMap<FGameplayTag, FSyEntityTagIndex> EntityTagMap; // GameplayTag -> 实体标签索引

    // 辅助函数，用于在注册/注销时更新TagMap
    void AddEntityToTagMap(USyEntityComponent* EntityComponent);
    void RemoveEntityFromTagMap(USyEntityComponent* EntityComponent);
};

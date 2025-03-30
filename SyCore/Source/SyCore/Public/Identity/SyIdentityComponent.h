#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "SyIdentityComponent.generated.h"

/**
 * SyIdentityComponent - 实体身份标识组件
 * 提供：
 * 1. 默认标签配置 - 在蓝图中配置
 * 2. 场景实例ID - 在场景实例化时生成
 * 3. 可选别名 - 可在蓝图或场景中配置
 */
UCLASS(Blueprintable, ClassGroup=(SyCore), meta=(BlueprintSpawnableComponent))
class SYCORE_API USyIdentityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USyIdentityComponent();

    // 基础查询接口
    UFUNCTION(BlueprintPure, Category = "SyIdentity")
    bool HasValidId() const { return EntityId.IsValid(); }

    UFUNCTION(BlueprintPure, Category = "SyIdentity")
    FGuid GetEntityId() const { return EntityId; }

    UFUNCTION(BlueprintPure, Category = "SyIdentity")
    bool HasTag(const FGameplayTag& Tag) const { return EntityTags.HasTag(Tag); }

    // 场景实例化时调用
    UFUNCTION(BlueprintCallable, Category = "SyIdentity")
    void GenerateEntityId();

protected:
    virtual void BeginPlay() override;

private:
    // 实例唯一标识
    UPROPERTY(VisibleAnywhere, Category = "SyIdentity")
    FGuid EntityId;

    // 可选别名 - 允许在编辑器中配置
    UPROPERTY(EditAnywhere, Category = "SyIdentity")
    FName EntityAlias;

    // 默认标签配置 - 在蓝图中配置
    UPROPERTY(EditDefaultsOnly, Category = "SyIdentity")
    FGameplayTagContainer EntityTags;

    // 事件委托 - 用于通知ID生成
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEntityIdGenerated);
    UPROPERTY(BlueprintAssignable, Category = "SyIdentity")
    FOnEntityIdGenerated OnEntityIdGenerated;
};
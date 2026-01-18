#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Components/ISyComponentInterface.h"
#include "SyIdentityComponent.generated.h"

/**
 * SyIdentityComponent - 实体身份标识组件
 * 提供：
 * 1. 默认标签配置 - 在蓝图中配置
 * 2. 场景实例ID - 在场景实例化时生成
 * 3. 可选别名 - 可在蓝图或场景中配置
 */
UCLASS(Blueprintable, ClassGroup=(SyCore), meta=(BlueprintSpawnableComponent))
class SYCORE_API USyIdentityComponent : public UActorComponent, public ISyComponentInterface
{
    GENERATED_BODY()

public:
    USyIdentityComponent();

    // 实现ISyComponentInterface接口
    virtual FName GetComponentType() const override { return TEXT("Identity"); }

    // 基础查询接口
    UFUNCTION(BlueprintPure, Category = "SyIdentity")
    bool HasValidId() const { return EntityId.IsValid(); }

    UFUNCTION(BlueprintPure, Category = "SyIdentity")
    FGuid GetEntityId() const { return EntityId; }

    UFUNCTION(BlueprintPure, Category = "SyIdentity")
    FGameplayTagContainer GetEntityTags() const { return EntityTags; }

    UFUNCTION(BlueprintPure, Category = "SyIdentity")
    FName GetEntityAlias() const { return EntityAlias; }
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

    // 可选别名
    UPROPERTY(EditAnywhere, Category = "SyIdentity")
    FName EntityAlias;

    // 默认标签配置
    // TODO: 只要更改参数就会触发实体强制重载，why baby why? 
    // 然后想让它在实例中不能编辑 EditDefaultOnly 和 VisibleAnywhere 冲突 
    // UE你是什么啥逼引擎？？？ 
    UPROPERTY(EditAnywhere, Category = "SyIdentity")
    FGameplayTagContainer EntityTags;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEntityIdGenerated);
    UPROPERTY(BlueprintAssignable, Category = "SyIdentity")
    FOnEntityIdGenerated OnEntityIdGenerated;

};

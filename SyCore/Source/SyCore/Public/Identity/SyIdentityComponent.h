#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "SyIdentityComponent.generated.h"

/**
 * SyIdentityComponent - 实体身份标识组件
 * 提供实体唯一标识、标签管理和别名系统
 */
UCLASS(Blueprintable, ClassGroup=(SyCore), meta=(BlueprintSpawnableComponent))
class SYCORE_API USyIdentityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USyIdentityComponent();

    // 基础接口
    UFUNCTION(BlueprintPure, Category = "SyIdentity")
    FGuid GetEntityId() const { return EntityId; }

    UFUNCTION(BlueprintPure, Category = "SyIdentity")
    FName GetEntityAlias() const { return EntityAlias; }

    UFUNCTION(BlueprintPure, Category = "SyIdentity")
    const FGameplayTagContainer& GetEntityTags() const { return EntityTags; }
    
    // 核心属性
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SyIdentity")
    FGuid EntityId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyIdentity")
    FName EntityAlias;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyIdentity")
    FGameplayTagContainer EntityTags;

    // 动态修改接口
    UFUNCTION(BlueprintCallable, Category = "SyIdentity")
    void SetEntityAlias(const FName& NewAlias);

    UFUNCTION(BlueprintCallable, Category = "SyIdentity")
    void AddEntityTag(const FGameplayTag& NewTag);

    UFUNCTION(BlueprintCallable, Category = "SyIdentity")
    void RemoveEntityTag(const FGameplayTag& TagToRemove);

    // 查询接口
    UFUNCTION(BlueprintPure, Category = "SyIdentity")
    bool HasTag(const FGameplayTag& Tag) const;

    UFUNCTION(BlueprintPure, Category = "SyIdentity")
    bool HasAnyTags(const FGameplayTagContainer& Tags) const;

    UFUNCTION(BlueprintPure, Category = "SyIdentity")
    bool HasAllTags(const FGameplayTagContainer& Tags) const;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // 生成唯一ID
    void GenerateEntityId();

    // 验证别名唯一性
    bool IsAliasUnique(const FName& Alias) const;

private:
    // 默认标签配置
    UPROPERTY(EditDefaultsOnly, Category = "SyIdentity")
    FGameplayTagContainer DefaultTags;

    // 调试支持
    UPROPERTY(EditAnywhere, Category = "SyIdentity|Debug")
    bool bShowDebugInfo;

    // 事件委托
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIdentityChanged, const USyIdentityComponent*, ChangedComponent);
    UPROPERTY(BlueprintAssignable, Category = "SyIdentity")
    FOnIdentityChanged OnIdentityChanged;
}; 
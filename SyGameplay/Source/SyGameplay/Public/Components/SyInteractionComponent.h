#pragma once

#include "CoreMinimal.h"
#include "Components/Base/InteractionComponent.h"
#include "SyCore/Public/Components/ISyComponentInterface.h"
#include "SyInteractionComponent.generated.h"

/**
 * SyInteractionComponent - 可交互组件
 * 职责：
 * 1. 继承自InteractionComponent，提供基础的交互功能
 * 2. 监听状态变化，根据"State.Interactable"标签控制交互能力
 * 3. 实现ISyComponentInterface接口，支持Sy系列组件的统一管理
 */
UCLASS(ClassGroup=(SyGameplay), meta=(BlueprintSpawnableComponent))
class SYGAMEPLAY_API USyInteractionComponent : public UInteractionComponent, public ISyComponentInterface
{
    GENERATED_BODY()

public:
    USyInteractionComponent();

    // 实现ISyComponentInterface接口
    virtual FName GetComponentType() const override { return TEXT("Interaction"); }

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    /** 缓存关联的StateComponent指针 */
    UPROPERTY(Transient)
    TObjectPtr<class USyStateComponent> StateComponent;

    /** 处理状态变化 */
    UFUNCTION()
    void HandleStateChanged();

    /** 查找并缓存StateComponent */
    void FindAndCacheStateComponent();
}; 
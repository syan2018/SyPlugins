#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SyPlayerInteractionManagerComponent.generated.h"

class UInteractionComponent;
class USyInteractionComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActiveInteractionChanged, USyInteractionComponent*, NewActiveInteraction);

/**
 * @brief 管理玩家可交互的组件，选择激活目标并处理交互请求。
 * @warning 当前使用静态事件订阅(UInteractionComponent::OnPlayerEnter/Exit)，未来应改为使用专用检测组件。
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SYGAMEPLAY_API USyPlayerInteractionManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USyPlayerInteractionManagerComponent();

    /** 当激活的交互组件改变时广播 */
    UPROPERTY(BlueprintAssignable, Category = "Interaction")
    FOnActiveInteractionChanged OnActiveInteractionChanged;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** 当前检测范围内所有潜在的可交互组件 (通过静态事件填充) */
    TArray<TWeakObjectPtr<USyInteractionComponent>> PotentialInteractions;

    /** 当前激活（通常是最近的）的可交互组件 */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interaction")
    TWeakObjectPtr<USyInteractionComponent> ActiveInteraction;

    // TODO: 未来应移除以下静态事件处理，改为由 InteractionDetectorComponent 驱动
    /** 处理交互组件进入范围的静态事件 */
    UFUNCTION()
    void HandleInteractionEnter(TWeakObjectPtr<UInteractionComponent> InteractionComponent);

    /** 处理交互组件离开范围的静态事件 */
    UFUNCTION()
    void HandleInteractionExit(TWeakObjectPtr<UInteractionComponent> InteractionComponent);
    // --- End of TODO section ---

    /** 更新激活的交互组件 */
    void UpdateActiveInteraction(float DeltaTime);

    /** 设置新的激活交互组件，并广播事件 */
    void SetActiveInteraction(USyInteractionComponent* NewInteraction);

public:
    /** 尝试执行当前激活交互组件的交互逻辑 */
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool TryExecuteActiveInteraction();

    /** 获取当前激活的交互组件 */
    UFUNCTION(BlueprintPure, Category = "Interaction")
    USyInteractionComponent* GetActiveInteraction() const;
};

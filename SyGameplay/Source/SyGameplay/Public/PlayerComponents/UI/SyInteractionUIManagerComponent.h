#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SyInteractionUIManagerComponent.generated.h"

class USyPlayerInteractionManagerComponent;
class USyInteractionComponent;
class UUserWidget;

/**
 * @brief 管理交互提示UI的显示与隐藏，监听玩家交互管理器的激活目标变化。
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SYGAMEPLAY_API USyInteractionUIManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USyInteractionUIManagerComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** 要显示的交互提示Widget类 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> InteractionWidgetClass;

    /** 当前显示的交互提示Widget实例 */
    UPROPERTY(Transient) // Transient表示这个变量不需要保存
    TObjectPtr<UUserWidget> CurrentInteractionWidget;

    /** 缓存对玩家交互管理器的引用 */
    UPROPERTY(Transient)
    TWeakObjectPtr<USyPlayerInteractionManagerComponent> InteractionManager;

    /** 处理激活交互组件变化的函数 */
    UFUNCTION()
    void HandleActiveInteractionChanged(USyInteractionComponent* NewActiveInteraction);

    /** 显示交互提示Widget */
    virtual void ShowInteractionWidget(USyInteractionComponent* InteractionComponent);

    /** 隐藏交互提示Widget */
    virtual void HideInteractionWidget();

    /** 更新Widget上的交互文本 (需要Widget实现特定接口或函数) */
    virtual void UpdateInteractionWidgetText(const FText& PromptText);

}; 
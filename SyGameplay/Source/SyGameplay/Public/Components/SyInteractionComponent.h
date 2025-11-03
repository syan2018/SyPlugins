#pragma once

#include "CoreMinimal.h"
#include "Components/Base/InteractionComponent.h"
#include "SyCore/Public/Components/ISyComponentInterface.h"
#include "GameplayTagContainer.h" // For FGameplayTag
#include "Metadatas/SyGameplayInteractValueTypes.h" // Include value types for delegate param
#include "SyInteractionComponent.generated.h"

// Forward declarations
class USyStateComponent;
class USyEntityComponent;
class UFlowComponent;
struct FInstancedStruct;
struct FSyBasicInteractInfo;

// Delegate signature for basic interaction
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSyOnBasicInteractionRequested, USyInteractionComponent*, InteractionComp);

/**
 * SyInteractionComponent - 可交互组件
 * 职责：
 * 1. 继承自InteractionComponent，提供基础的交互功能
 * 2. 监听状态变化，根据"State.Interactable"标签控制交互能力
 * 3. 读取"State.Interact.InteractInfo"元数据，执行第一个交互项定义的逻辑
 * 4. 为Basic类型交互提供蓝图事件
 * 5. 为Flow类型交互自动处理FlowComponent并启动Flow
 * 6. 实现ISyComponentInterface接口，支持Sy系列组件的统一管理
 */
UCLASS(ClassGroup=(SyGameplay), meta=(BlueprintSpawnableComponent))
class SYGAMEPLAY_API USyInteractionComponent : public UInteractionComponent, public ISyComponentInterface
{
    GENERATED_BODY()

public:
    USyInteractionComponent();

    // --- ISyComponentInterface --- 
    virtual FName GetComponentType() const override { return TEXT("Interaction"); }
    
    // InteractionComponent 在功能阶段初始化
    virtual ESyComponentInitPhase GetInitializationPhase() const override 
    { 
        return ESyComponentInitPhase::Functional; 
    }
    
    // 由 EntityComponent 调用的初始化方法
    virtual void OnSyComponentInitialized() override;

    // --- Basic Interaction Event --- 
    /** 当执行的交互类型为 Basic 时广播 */
    UPROPERTY(BlueprintAssignable, Category = "SyGameplay|Interaction")
    FSyOnBasicInteractionRequested OnBasicInteractionRequested;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** GameplayTag 用于从 StateComponent 获取交互列表元数据 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SyGameplay|Interaction")
    FGameplayTag InteractionListMetadataTag = FGameplayTag::RequestGameplayTag(FName("State.Interact.InteractInfo"));

private:
    /** 缓存关联的 StateComponent 指针 */
    UPROPERTY(Transient)
    TObjectPtr<USyStateComponent> CachedStateComponent;

    /** 缓存关联的 EntityComponent 指针 */
    UPROPERTY(Transient)
    TObjectPtr<USyEntityComponent> CachedEntityComponent;

    /** 查找并缓存关联 SyComponent */
    void FindAndCacheComponents();

    /** 处理状态变化，根据 State.Interactable 更新交互能力 */
    UFUNCTION()
    void HandleStateChanged();

    /** 
     * 处理交互请求 (绑定到 OnUsed 事件)
     * 读取交互列表元数据，执行第一个交互项的逻辑。
     */
    UFUNCTION()
    void HandleInteractionRequest();

    UFUNCTION()
    void HandleInteractionFinish() const;

    /** 辅助函数：处理 Flow 类型的交互 */
    void ProcessFlowInteraction(const FInstancedStruct& InteractionInfoStruct);

    /** 辅助函数：处理 Basic 类型的交互 */
    void ProcessBasicInteraction(const FInstancedStruct& InteractionInfoStruct);
}; 
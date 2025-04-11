#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "SyStateCore.h" 
#include "SyStateManager/Public/StateModificationRecord.h" // 包含 FSyStateModificationRecord
#include "SyStateComponent.generated.h"

// 前向声明
class USyStateManagerSubsystem;
struct FSyStateParameterSet; 

/**
 * SyStateComponent - 实体状态组件
 * 职责：
 * 1. 使用 FSyStateParameterSet 进行初始化。
 * 2. 维护实体的本地运行时状态 (FSyStateCategories)。
 * 3. 提供状态查询接口 (通过 FSyStateCategories)。
 * 4. 连接到全局 USyStateManagerSubsystem，监听状态修改记录。
 * 5. 应用与其相关的状态修改记录到本地状态。
 * 6. 触发本地状态数据变更事件 (TODO)。
 */
UCLASS(ClassGroup=(SyEntity), meta=(BlueprintSpawnableComponent))
class SYENTITY_API USyStateComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USyStateComponent();

    // --- 状态访问 ---
    /**
     * @brief 获取实体当前的运行时状态容器。
     * @return 对本地 FSyStateCategories 的常量引用。
     * @note 直接通过此容器访问状态数据 (例如使用 FindFirstStateMetadata<T>)。
     */
    UFUNCTION(BlueprintPure, Category = "SyState")
    const FSyStateCategories& GetCurrentStateCategories() const { return CurrentStateCategories; }

    // --- 配置 ---
    /**
     * @brief 实体状态的初始化数据。
     *        可以在蓝图编辑器中直接配置，或者在 Actor 构造时动态设置。
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyState|Config", meta=(DisplayName="Default Initialization Data"))
    FSyStateParameterSet DefaultInitData;

    /**
     * @brief 手动应用初始化数据。
     *        通常在 BeginPlay 中自动调用一次 DefaultInitData。
     *        如果需要在运行时重新初始化或应用不同的初始化数据，可以调用此函数。
     * @param InitData 要应用的初始化数据。
     */
    UFUNCTION(BlueprintCallable, Category = "SyState|Initialization")
    void ApplyInitializationData(const FSyStateParameterSet& InitData);

    /**
     * @brief 获取本组件关联的目标类型标签。
     *        StateManager 会使用此标签来初步筛选相关的操作记录。
     *        建议在派生类或 Actor 实例中设置。
     */
    UFUNCTION(BlueprintPure, Category = "SyState|Config")
    FGameplayTag GetTargetTypeTag() const { return TargetTypeTag; }

    /**
     * @brief 设置本组件关联的目标类型标签。
     */
    UFUNCTION(BlueprintCallable, Category = "SyState|Config")
    void SetTargetTypeTag(const FGameplayTag& NewTag);

    /**
     * @brief 是否启用与全局 StateManager 的同步
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyState|Config", meta=(DisplayName="Enable Global Sync"))
    bool bEnableGlobalSync = true;

    // TODO: [拓展] 本地状态变更事件 
    /** 当本地状态数据实际发生变化时广播。
     *  注意：这与 StateManager 的记录事件不同，这个事件表示本地状态数据已被修改。
     *  参数可以设计为传递变化的具体 StateTag 和对应的 Metadata 对象，或者更简单的只通知"状态已变"。
     *  为了简化，我们先定义一个无参数的委托。
     */
    DECLARE_MULTICAST_DELEGATE(FOnLocalStateDataChanged);
    FOnLocalStateDataChanged OnLocalStateDataChanged;

protected:
    //~ Begin UActorComponent Interface
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //~ End UActorComponent Interface

    /** 存储实体当前的运行时状态 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SyState|Data", meta = (AllowPrivateAccess = "true"))
    FSyStateCategories CurrentStateCategories;

    /** 本组件对应的目标类型标签，用于 StateManager 筛选 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SyState|Config", meta = (AllowPrivateAccess = "true"))
    FGameplayTag TargetTypeTag;

private:
    /** 缓存 StateManager 子系统指针 */
    UPROPERTY(Transient)
    TObjectPtr<USyStateManagerSubsystem> StateManagerSubsystem;

    /**
     * @brief 处理从 StateManager 接收到的新状态修改记录。
     * @param NewRecord 新记录。
     */
    UFUNCTION()
    void HandleStateModificationRecorded(const FSyStateModificationRecord& NewRecord);

    /**
     * @brief 尝试连接到 StateManager 并订阅事件。
     */
    void TryConnectToStateManager();

    /**
     * @brief 从 StateManager 断开连接并取消订阅。
     */
    void DisconnectFromStateManager();

    /**
     * @brief 应用聚合后的状态修改到本地状态。
     *        内部会调用 FSyStateCategories 的 ApplyStateModifications 方法。
     */
    void ApplyAggregatedModifications();

    // TODO: [拓展] 可能需要一个 Entity ID (FGuid) 来更精确地匹配 StateManager 中的目标。
    // 这个 ID 可以由外部系统设置，或者自动生成。
    // UPROPERTY(...) FGuid EntityId;
}; 
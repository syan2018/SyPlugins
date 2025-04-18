#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "SyStateCore.h" 
#include "SyEntityComponent.h"
#include "SyStateManager/Public/StateModificationRecord.h" // 包含 FSyStateModificationRecord
#include "SyCore/Public/Components/ISyComponentInterface.h"
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
class SYENTITY_API USyStateComponent : public UActorComponent, public ISyComponentInterface
{
    GENERATED_BODY()

public:
    USyStateComponent();

    // 实现ISyComponentInterface接口
    virtual FName GetComponentType() const override { return TEXT("State"); }

    // --- 状态访问 ---
    /**
     * @brief 获取本地（初始/默认）状态。
     * @return 对本地 FSyStateCategories 的常量引用。
     * @note 直接通过此容器访问状态数据 (例如使用 FindFirstStateMetadata<T>)。
     */
    UFUNCTION(BlueprintPure, Category = "SyState|Access")
    const FSyStateCategories& GetLocalStateCategories() const { return LocalStateCategories; }

    /**
     * @brief 获取全局（聚合修改后）的状态。
     * @return 对全局 FSyStateCategories 的常量引用。
     * @note 直接通过此容器访问状态数据 (例如使用 FindFirstStateMetadata<T>)。
     */
    UFUNCTION(BlueprintPure, Category = "SyState|Access")
    const FSyStateCategories& GetGlobalStateCategories() const { return GlobalStateCategories; }

    /**
     * @brief 获取最终生效的状态集合 (本地状态被全局状态覆盖/合并)
     * @return 一个包含最终生效状态的 FSyStateCategories 副本。
     */
    UFUNCTION(BlueprintPure, Category = "SyState|Access", meta=(DisplayName="Get Effective State Categories"))
    FSyStateCategories GetEffectiveStateCategories() const;

    /**
     * @brief 获取指定标签最终生效的第一个元数据参数。
     * 优先从全局状态查找，如果找不到则从本地状态查找。
     * @param StateTag 要查找的状态标签。
     * @param OutParam 如果找到，将填充参数；否则保持不变。
     * @return 如果找到参数则返回 true。
     */
    UFUNCTION(BlueprintCallable, Category = "SyState|Access", meta=(DisplayName="Get Effective State Param (Struct)"))
    bool GetEffectiveStateParam(FGameplayTag StateTag, UPARAM(ref) FInstancedStruct& OutParam) const;

    /**
     * @brief 获取指定标签最终生效的第一个特定类型的元数据值。
     * @tparam T 期望获取的元数据内部值的类型 (e.g., bool, float, FVector)。
     * @param StateTag 要查找的状态标签。
     * @param OutValue 如果找到且类型匹配，则填充值。
     * @return 如果成功找到并获取到正确类型的值，返回 true。
     */
    template<typename T>
    bool GetEffectiveStateValue(FGameplayTag StateTag, T& OutValue) const;

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
     *        从关联的EntityComponent中获取第一个Tag作为目标类型标签。
     *        如果没有关联的EntityComponent或没有Tag，则返回无效标签。
     */
    UFUNCTION(BlueprintPure, Category = "SyState|Config")
    FGameplayTag GetTargetTypeTag() const;

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
    DECLARE_MULTICAST_DELEGATE(FOnEffectiveStateChanged);
    FOnEffectiveStateChanged OnEffectiveStateChanged;

protected:
    //~ Begin UActorComponent Interface
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //~ End UActorComponent Interface

    /** 存储本地（初始/默认）状态数据 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SyState|Internal", meta = (AllowPrivateAccess = "true"))
    FSyStateCategories LocalStateCategories;

    /** 存储从 StateManager 聚合来的全局状态修改 */
    UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "SyState|Internal", meta = (AllowPrivateAccess = "true"))
    FSyStateCategories GlobalStateCategories;

private:
    /** 缓存 StateManager 子系统指针 */
    UPROPERTY(Transient)
    TObjectPtr<USyStateManagerSubsystem> StateManagerSubsystem;

    /** 缓存关联的EntityComponent指针 */
    UPROPERTY(Transient)
    TObjectPtr<USyEntityComponent> EntityComponent;

    /**
     * @brief 处理从 StateManager 接收到的新状态修改记录。
     * @param NewRecord 新记录。
     */
    UFUNCTION()
    void HandleStateModificationChanged(const FSyStateModificationRecord& ChangedRecord);

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

    /**
     * @brief 查找并缓存关联的EntityComponent
     */
    void FindAndCacheEntityComponent();

    // TODO: [拓展] 可能需要一个 Entity ID (FGuid) 来更精确地匹配 StateManager 中的目标。
    // 这个 ID 可以由外部系统设置，或者自动生成。
    // UPROPERTY(...) FGuid EntityId;
};

// Template implementation for GetEffectiveStateValue
template<typename T>
bool USyStateComponent::GetEffectiveStateValue(FGameplayTag StateTag, T& OutValue) const
{
    FInstancedStruct ParamStruct;
    if (GetEffectiveStateParam(StateTag, ParamStruct))
    {
        if (const T* ValuePtr = ParamStruct.GetPtr<T>())
        {
            OutValue = *ValuePtr;
            return true;
        }
        // Optional: Log type mismatch warning?
    }
    return false;
} 
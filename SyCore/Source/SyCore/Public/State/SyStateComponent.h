#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Entity/SyEntityComponent.h"
#include "State/StateModificationRecord.h" // 包含 FSyStateModificationRecord
#include "Foundation/ISyComponentInterface.h"
#include "Types/StateContainerTypes.h"
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
class SYCORE_API USyStateComponent : public UActorComponent, public ISyComponentInterface
{
    GENERATED_BODY()

public:
    USyStateComponent();

    // 实现ISyComponentInterface接口
    virtual FName GetComponentType() const override { return TEXT("State"); }
    
    // StateComponent 在核心阶段初始化（最先）
    virtual ESyComponentInitPhase GetInitializationPhase() const override 
    { 
        return ESyComponentInitPhase::Core; 
    }
    
    // 由 EntityComponent 调用，广播初始状态
    virtual void OnSyComponentInitialized() override;

    // --- 状态访问 ---
    /**
     * @brief 获取指定层级的状态
     * @param Layer 状态层级
     * @return 对该层级状态的常量引用
     */
    UFUNCTION(BlueprintPure, Category = "SyState|Access")
    const FSyStateCategories& GetStateLayer(ESyStateLayer Layer) const;

    /**
     * @brief 获取最终生效的状态集合 (所有层级合并后的结果)
     * @return 一个包含最终生效状态的 FSyStateCategories 副本。
     * @note 使用缓存机制，性能已优化
     */
    UFUNCTION(BlueprintPure, Category = "SyState|Access", meta=(DisplayName="Get Effective State Categories"))
    FSyStateCategories GetEffectiveStateCategories() const;

    /**
     * @brief 获取分层状态容器的直接访问
     * @return 对分层状态容器的常量引用
     * @note 高级用法，用于需要直接操作层级的场景
     */
    const FSyLayeredStateContainer& GetLayeredStateContainer() const { return LayeredState; }

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
     * @brief 手动应用初始化数据（应用到 Default 层）。
     *        通常在 BeginPlay 中自动调用一次 DefaultInitData。
     *        如果需要在运行时重新初始化或应用不同的初始化数据，可以调用此函数。
     * @param InitData 要应用的初始化数据。
     */
    UFUNCTION(BlueprintCallable, Category = "SyState|Initialization")
    void ApplyInitializationData(const FSyStateParameterSet& InitData);

    /**
     * @brief 应用临时状态修改（Buff/Debuff等）到 Temporary 层
     * @param TempModifications 临时修改的参数集
     */
    UFUNCTION(BlueprintCallable, Category = "SyState|Modification")
    void ApplyTemporaryModifications(const FSyStateParameterSet& TempModifications);

    /**
     * @brief 清除指定层级的所有状态
     * @param Layer 要清除的层级
     */
    UFUNCTION(BlueprintCallable, Category = "SyState|Modification")
    void ClearStateLayer(ESyStateLayer Layer);

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

    /** 分层状态容器 - 使用层级系统管理状态
     *  - Default 层：初始化数据
     *  - Persistent 层：从 StateManager 同步的全局状态
     *  - Temporary 层：临时修改（Buff等）
     *  - Override 层：强制覆盖
     */
    UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "SyState|Internal")
    FSyLayeredStateContainer LayeredState;

private:
    /** 缓存 StateManager 子系统指针 */
    UPROPERTY(Transient)
    TObjectPtr<USyStateManagerSubsystem> StateManagerSubsystem;

    /** 缓存关联的EntityComponent指针 */
    UPROPERTY(Transient)
    TObjectPtr<USyEntityComponent> EntityComponent;

    /** 标记状态组件是否已完全初始化（包括默认数据和全局同步） */
    bool bIsFullyInitialized = false;

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

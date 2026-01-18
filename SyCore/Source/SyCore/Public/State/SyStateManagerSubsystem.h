// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Delegates/DelegateCombinations.h"
#include "State/StateModificationRecord.h"
#include "State/Types/StateParameterTypes.h"
#include "GameplayTagContainer.h"
#include "Templates/Function.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "State/Operations/OperationTypes.h" // Needed for FSyOperationSource in new functions
#include "SyStateManagerSubsystem.generated.h"

// Forward declaration FSyOperation
struct FSyOperation;

/**
 * @brief 状态修改记录发生变化（添加或移除）时的委托
 * @param ChangedRecord 发生变化的记录
 */
// 动态多播委托 - 用于蓝图和全局广播
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateModificationChanged, const FSyStateModificationRecord&, ChangedRecord);

// 普通委托 - 用于 C++ 智能订阅（支持 Bind 和 Execute）
DECLARE_DELEGATE_OneParam(FOnStateModificationChangedNative, const FSyStateModificationRecord&);

/**
 * @brief 状态管理器子系统 (Game Instance Subsystem)
 * 负责记录和移除状态变更操作意图 (FSyOperation) 并广播相关事件。
 * 它不直接修改任何状态，仅作为操作日志和通知中心。
 * 支持将会话期间的操作日志持久化到存档。
 */
UCLASS()
class SYCORE_API USyStateManagerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    //~ Begin USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //~ End USubsystem Interface

    // --- Recording Operations --- 

    /**
     * @brief 记录一个操作意图。成功记录后会广播 OnStateModificationChanged 事件。
     * @param Operation 要记录的操作。Operation.OperationId 应由调用者生成并保证唯一性。
     * @return 如果记录成功，返回 true。
     */
    UFUNCTION(BlueprintCallable, Category="State Management", meta=(DisplayName="Record Operation Intent"))
    virtual bool RecordOperation(const FSyOperation& Operation);

    // --- Unloading Operations --- 

    /**
     * @brief 通过 Operation ID 卸载（移除）一个已记录的操作意图。成功移除后会广播 OnStateModificationChanged 事件。
     * @param OperationIdToUnload 要卸载的操作的唯一 ID。
     * @return 如果找到了匹配的 ID 并成功移除了记录，返回 true。
     */
    UFUNCTION(BlueprintCallable, Category="State Management", meta=(DisplayName="Unload Operation by ID"))
    virtual bool UnloadOperation(const FGuid& OperationIdToUnload);

    /**
     * @brief 通过匹配源信息卸载（移除）一个或多个已记录的操作意图。每个成功移除的记录都会广播 OnStateModificationChanged 事件。
     * @param SourceToMatch 要匹配的源信息。
     * @return 成功移除的记录数量。
     * @note 匹配逻辑基于 FSyOperationSource 的相等性比较。
     */
    UFUNCTION(BlueprintCallable, Category="State Management", meta=(DisplayName="Unload Operations by Source"))
    virtual int32 UnloadOperationsBySource(const FSyOperationSource& SourceToMatch);

    // --- Querying Operations --- 

    /**
     * @brief 获取聚合后的状态修改参数集
     * @param TargetFilterTag 用于筛选操作的目标类型标签。只有 Target.TargetTypeTag 与此匹配的操作才会被考虑。
     *                        如果传入无效 Tag，则不进行目标类型筛选。
     * @return 一个 FSyStateParameterSet，包含了所有通过筛选的操作记录中 Modifier 的 StateModifications 的聚合结果。
     */
    UFUNCTION(BlueprintCallable, Category="State Management", meta=(DisplayName="Get Aggregated Modifications"))
    virtual FSyStateParameterSet GetAggregatedModifications(const FGameplayTag& TargetFilterTag /* TODO: 添加 SourceFilterTag */) const;

    /**
     * @brief 获取所有已记录的修改 (简单版本)
     * @return 日志中所有记录的常量引用。
     */
    UFUNCTION(BlueprintPure, Category="State Management", meta=(DisplayName="Get All Modifications (Simple)"))
    virtual const TArray<FSyStateModificationRecord>& GetAllModifications_Simple() const;

    // --- Events --- 

    /** 当状态修改被记录或卸载时广播（全局事件，用于蓝图或需要监听所有变更的场景）
     *  C++ 代码建议使用 SubscribeToTargetType 进行精准订阅以获得更好的性能
     */
    UPROPERTY(BlueprintAssignable, Category="State Management|Events", meta=(DisplayName="On State Modification Changed"))
    FOnStateModificationChanged OnStateModificationChanged;
    
    // ===== 智能订阅接口 =====
    
    /**
     * @brief 订阅特定目标类型的状态修改（C++ 使用）
     * @param TargetTypeTag 要订阅的目标类型标签
     * @param Subscriber 订阅者对象
     * @param Delegate 回调委托（普通委托，支持 Bind）
     */
    void SubscribeToTargetType(
        FGameplayTag TargetTypeTag, 
        UObject* Subscriber,
        FOnStateModificationChangedNative Delegate);
    
    /**
     * @brief 取消订阅特定目标类型的状态修改
     * @param TargetTypeTag 目标类型标签
     * @param Subscriber 订阅者对象
     */
    UFUNCTION(BlueprintCallable, Category="State Management|Subscription")
    void UnsubscribeFromTargetType(FGameplayTag TargetTypeTag, UObject* Subscriber);
    
    /**
     * @brief 取消订阅者的所有订阅
     * @param Subscriber 订阅者对象
     */
    UFUNCTION(BlueprintCallable, Category="State Management|Subscription")
    void UnsubscribeAll(UObject* Subscriber);

    // --- Persistence --- 

    /**
     * @brief 将当前的操作日志保存到存档文件
     * @return 如果保存成功，返回true。
     */
    UFUNCTION(BlueprintCallable, Category="State Management|Persistence")
    bool SaveLog();

    /**
     * @brief 从存档文件加载操作日志
     * @return 如果加载成功，返回true。
     */
    UFUNCTION(BlueprintCallable, Category="State Management|Persistence")
    bool LoadLog();

    // TODO: [拓展] 查询优化接口
    // TODO: [拓展] 网络同步支持

private:
    /** 存储所有状态修改记录的日志 (运行时 + 从存档加载) */
    UPROPERTY(Transient)
    TArray<FSyStateModificationRecord> ModificationLog;
    
    // ===== 性能优化：索引和缓存 =====
    
    /** 按目标类型索引的记录 - 加速查询 
     *  注意：不能使用 UPROPERTY，因为 UE 不支持嵌套容器 TMap<Tag, TArray<int32>>
     */
    TMap<FGameplayTag, TArray<int32>> TargetTypeIndex;
    
    /** 按操作ID索引的记录 - 加速卸载操作 */
    TMap<FGuid, int32> OperationIdIndex;
    
    /** 聚合结果缓存 - 避免重复计算 
     *  注意：不能使用 UPROPERTY，因为缓存是临时数据且包含复杂类型
     */
    TMap<FGameplayTag, FSyStateParameterSet> AggregatedCache;
    
    /** 缓存版本号 - 用于缓存失效 */
    TMap<FGameplayTag, int32> CacheVersions;
    
    /** 全局版本号 - 每次记录操作时递增 */
    int32 GlobalVersion = 0;
    
    // ===== 智能订阅数据结构 =====
    
    /** 订阅者信息 */
    struct FSubscriberInfo
    {
        TWeakObjectPtr<UObject> Subscriber;
        FOnStateModificationChangedNative Delegate;  // 使用普通委托
        
        FSubscriberInfo() = default;
        FSubscriberInfo(UObject* InSubscriber, FOnStateModificationChangedNative InDelegate)
            : Subscriber(InSubscriber)
            , Delegate(InDelegate)
        {}
        
        bool IsValid() const { return Subscriber.IsValid() && Delegate.IsBound(); }
    };
    
    /** 按目标类型分组的订阅者 - 精准广播 */
    TMap<FGameplayTag, TArray<FSubscriberInfo>> TargetTypeSubscribers;

    /** 定义存档槽位名称 */
    inline static const FString SaveSlotName = TEXT("SyStateManagerLog");
    /** 定义存档用户索引 (通常为0) */
    inline static const int32 UserIndex = 0;

    /**
     * @brief 添加记录到日志并广播 OnStateModificationChanged 事件
     * @param Record 要添加的记录
     */
    void AddRecordAndBroadcast(const FSyStateModificationRecord& Record);

    /**
     * @brief 对传入的操作进行基础验证 (可选)
     * @param Operation 待验证的操作
     * @return 如果操作有效，返回true。
     */
    virtual bool ValidateOperation(const FSyOperation& Operation) const;
    
    /**
     * @brief 聚合单个记录的修改到输出Map中（内部辅助方法）
     * @param Record 要聚合的记录
     * @param OutAggregatedMap 输出的聚合Map
     */
    void AggregateRecordModifications(
        const FSyStateModificationRecord& Record,
        TMap<FGameplayTag, TArray<FInstancedStruct>>& OutAggregatedMap) const;

    /**
     * @brief 重新计算指定目标类型的聚合快照
     * @param TargetTag 目标类型标签
     * @note 用于卸载操作后刷新快照
     */
    void RecalculateSnapshotForTarget(const FGameplayTag& TargetTag);
    
    /**
     * @brief 精准广播状态修改事件给相关订阅者
     * @param Record 状态修改记录
     */
    void BroadcastToSubscribers(const FSyStateModificationRecord& Record);
    
    /**
     * @brief 清理无效的订阅者（在订阅列表中定期调用）
     */
    void CleanupInvalidSubscribers();

    // TODO: [拓展] 日志管理
    // - 日志大小限制与清理策略
    // - 考虑更频繁或更智能的保存时机（例如，定期、特定事件触发）
};

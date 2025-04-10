// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Delegates/DelegateCombinations.h"
#include "StateModificationRecord.h"
#include "StateParameterTypes.h"
#include "GameplayTagContainer.h"
#include "Templates/Function.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SyStateManagerSubsystem.generated.h"

// 前向声明 FSyOperation
struct FSyOperation;

/**
 * @brief 状态修改记录被添加时的委托
 * @param NewRecord 新添加的记录
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateModificationRecorded, const FSyStateModificationRecord&, NewRecord);

/**
 * @brief 状态管理器子系统 (Game Instance Subsystem)
 * 负责记录状态变更操作意图 (FSyOperation) 并广播相关事件。
 * 它不直接修改任何状态，仅作为操作日志和通知中心。
 * 支持将会话期间的操作日志持久化到存档。
 */
UCLASS()
class SYSTATEMANAGER_API USyStateManagerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    //~ Begin USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //~ End USubsystem Interface

    /**
     * @brief 记录一个操作意图
     * @param Operation 要记录的操作
     * @return 如果记录成功（例如通过了基本的验证），返回true。
     */
    UFUNCTION(BlueprintCallable, Category="State Management", meta=(DisplayName="Record Operation Intent"))
    virtual bool RecordOperation(const FSyOperation& Operation);

    /**
     * @brief 获取聚合后的状态修改参数集
     * @param TargetFilterTag 用于筛选操作的目标类型标签。只有 Target.TargetTypeTag 与此匹配的操作才会被考虑。
     *                        如果传入无效 Tag (例如 FGameplayTag::EmptyTag)，可以考虑匹配所有目标（待定）。
     * @return 一个 FSyStateParameterSet，包含了所有通过筛选的操作记录中 Modifier 的 StateModifications 的聚合结果。
     * @note 此函数负责根据提供的筛选条件，从日志中提取相关的状态修改意图并聚合成一个方便下游应用的参数集。
     */
    UFUNCTION(BlueprintCallable, Category="State Management", meta=(DisplayName="Get Aggregated Modifications"))
    virtual FSyStateParameterSet GetAggregatedModifications(const FGameplayTag& TargetFilterTag /* TODO: 添加 SourceFilterTag */) const;

    /**
     * @brief 获取所有已记录的修改 (简单版本)
     * @return 日志中所有记录的常量引用。
     * @note 主要用于调试或需要完整日志的场景。
     */
    UFUNCTION(BlueprintPure, Category="State Management", meta=(DisplayName="Get All Modifications (Simple)"))
    virtual const TArray<FSyStateModificationRecord>& GetAllModifications_Simple() const;

    /** 当有新的状态修改被记录时广播 */
    UPROPERTY(BlueprintAssignable, Category="State Management|Events", meta=(DisplayName="On State Modification Recorded"))
    FOnStateModificationRecorded OnStateModificationRecorded;

    // TODO: [拓展] 优化订阅接口 - 允许组件根据更精细的条件订阅，由StateManager进行过滤
    /**
    struct FSyTargetFilter // 示例过滤器结构体
    {
        FGameplayTag TargetType;
        FGuid TargetEntityId; // 可选
        // ... 其他过滤条件 ...
    };
    DECLARE_DELEGATE_OneParam(FStateModificationDelegate, const FSyStateModificationRecord&);
    virtual FDelegateHandle SubscribeByTarget(const FSyTargetFilter& Filter, FStateModificationDelegate Callback);
    virtual void UnsubscribeByTarget(FDelegateHandle Handle);
    **/

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

    /** 定义存档槽位名称 */
    inline static const FString SaveSlotName = TEXT("SyStateManagerLog");
    /** 定义存档用户索引 (通常为0) */
    inline static const int32 UserIndex = 0;

    /**
     * @brief 添加记录到日志并广播事件
     * @param Record 要添加的记录
     */
    void AddRecordAndBroadcast(const FSyStateModificationRecord& Record);

    /**
     * @brief 对传入的操作进行基础验证 (可选)
     * @param Operation 待验证的操作
     * @return 如果操作有效，返回true。
     */
    virtual bool ValidateOperation(const FSyOperation& Operation) const;

    // TODO: [拓展] 日志管理
    // - 日志大小限制与清理策略
    // - 性能优化 (索引等)
    // - 考虑更频繁或更智能的保存时机（例如，定期、特定事件触发）
}; 
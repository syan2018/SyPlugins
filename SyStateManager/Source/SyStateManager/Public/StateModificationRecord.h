// StateModificationRecord.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Misc/Guid.h"
#include "Misc/DateTime.h"
#include "OperationTypes.h" // 引入 FSyOperation 的定义 (需要确保 SyOperation 模块是依赖项)
#include "StateModificationRecord.generated.h"

/**
 * @brief 状态修改记录
 * 封装一个已被状态管理器记录的操作 (FSyOperation) 及其相关元数据。
 * 这个结构体保存了操作的完整信息，允许下游系统根据需求灵活地筛选和处理。
 */
USTRUCT(BlueprintType)
struct SYSTATEMANAGER_API FSyStateModificationRecord
{
    GENERATED_BODY()

    /** 此记录自身的唯一标识符 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Record", meta=(Comment="此记录自身的唯一标识符"))
    FGuid RecordId;

    /** 此记录被创建的时间戳 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Record", meta=(Comment="此记录被创建的时间戳"))
    FDateTime Timestamp;

    /** 实际发生并被记录的操作的完整数据 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Record", meta=(Comment="实际发生并被记录的操作的完整数据"))
    FSyOperation Operation;

    // 默认构造函数
    FSyStateModificationRecord()
        : RecordId(FGuid::NewGuid()), // 创建时自动生成ID
          Timestamp(FDateTime::UtcNow())
    {}

    // 从 FSyOperation 构造，方便使用
    explicit FSyStateModificationRecord(const FSyOperation& InOperation)
        : RecordId(FGuid::NewGuid()),
          Timestamp(FDateTime::UtcNow()),
          Operation(InOperation) // 直接拷贝操作数据
    {}

    // TODO: [拓展] 可以考虑添加额外的元数据，例如：
    // - 操作是被哪个系统或流程发起的？(FGameplayTag SourceSystemTag?)
    // - 操作执行的结果状态？(ESyOperationResult ResultStatus?) - 但这会增加StateManager的职责，需要谨慎考虑
}; 
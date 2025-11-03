#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "SyMessageTypes.generated.h"

// 消息优先级
UENUM(BlueprintType)
enum class ESyMessagePriority : uint8
{
    /** 低优先级 - 可以延迟处理 */
    Low UMETA(DisplayName = "Low"),
    
    /** 普通优先级 - 正常队列处理 */
    Normal UMETA(DisplayName = "Normal"),
    
    /** 高优先级 - 优先处理 */
    High UMETA(DisplayName = "High"),
    
    /** 立即处理 - 不进入队列 */
    Immediate UMETA(DisplayName = "Immediate")
};

// 消息源结构
USTRUCT(BlueprintType)
struct SYCORE_API FSyMessageSource
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message|Source")
    FGameplayTag SourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message|Source")
    FGuid SourceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message|Source")
    FName SourceAlias;
};

// 消息内容结构（增强版）
USTRUCT(BlueprintType)
struct SYCORE_API FSyMessageContent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message|Content")
    FGameplayTag MessageType;

    /** 消息负载 - 支持任意类型的结构化数据 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message|Content")
    FInstancedStruct Payload;

    /** 元数据 - 保留用于简单的键值对（向后兼容） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message|Content")
    TMap<FName, FString> Metadata;
    
    /** 辅助方法：尝试获取特定类型的 Payload */
    template<typename T>
    bool TryGetPayload(T& OutPayload) const
    {
        if (const T* PayloadPtr = Payload.GetPtr<T>())
        {
            OutPayload = *PayloadPtr;
            return true;
        }
        return false;
    }
    
    /** 辅助方法：设置 Payload */
    template<typename T>
    void SetPayload(const T& InPayload)
    {
        Payload.InitializeAs<T>(InPayload);
    }
};

// 完整消息结构（增强版）
USTRUCT(BlueprintType)
struct SYCORE_API FSyMessage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
    FSyMessageSource Source;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
    FSyMessageContent Content;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
    FDateTime Timestamp;
    
    /** 消息优先级 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
    ESyMessagePriority Priority = ESyMessagePriority::Normal;
    
    /** 消息唯一ID（用于追踪和去重） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Message")
    FGuid MessageId;
    
    /** 默认构造函数 - 生成唯一ID和时间戳 */
    FSyMessage()
        : Timestamp(FDateTime::Now())
        , Priority(ESyMessagePriority::Normal)
        , MessageId(FGuid::NewGuid())
    {}
}; 
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SyMessageTypes.h"
#include "SyMessageReceiver.h"
#include "SyMessageFilter.h"
#include "SyMessageBus.generated.h"

/**
 * 消息总线 - 负责消息的分发和订阅管理（增强版）
 * 1. 管理全局消息订阅
 * 2. 处理消息分发（支持优先级）
 * 3. 提供Flow节点所需的订阅接口
 * 4. 消息历史记录
 * 5. 按消息类型的智能订阅
 */
UCLASS()
class SYCORE_API USyMessageBus : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    //~ Begin USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //~ End USubsystem Interface
    
    // ===== 基础消息广播 =====
    
    /**
     * @brief 广播消息（支持优先级）
     * @param Message 要广播的消息
     */
    UFUNCTION(BlueprintCallable, Category = "Message Bus")
    void BroadcastMessage(const FSyMessage& Message);
    
    /**
     * @brief 广播消息并指定优先级
     * @param Message 要广播的消息
     * @param Priority 消息优先级
     */
    UFUNCTION(BlueprintCallable, Category = "Message Bus")
    void BroadcastMessageWithPriority(FSyMessage Message, ESyMessagePriority Priority);

    // ===== Filter 订阅接口（保持兼容） =====
    
    /** Flow节点订阅接口 */
    void SubscribeWithFilter(USyMessageFilterComposer* Filter, UObject* Subscriber);
    void UnsubscribeWithFilter(USyMessageFilterComposer* Filter, UObject* Subscriber);
    TArray<UObject*> GetSubscribersForFilter(USyMessageFilterComposer* Filter) const;
    
    // ===== 智能订阅接口（新增） =====
    
    /**
     * @brief 订阅特定消息类型
     * @param MessageType 要订阅的消息类型标签
     * @param Subscriber 订阅者对象
     * @param Callback 回调函数
     */
    UFUNCTION(BlueprintCallable, Category = "Message Bus|Subscription")
    void SubscribeToMessageType(FGameplayTag MessageType, UObject* Subscriber);
    
    /**
     * @brief 取消订阅消息类型
     * @param MessageType 消息类型标签
     * @param Subscriber 订阅者对象
     */
    UFUNCTION(BlueprintCallable, Category = "Message Bus|Subscription")
    void UnsubscribeFromMessageType(FGameplayTag MessageType, UObject* Subscriber);
    
    /**
     * @brief 取消订阅者的所有订阅
     * @param Subscriber 订阅者对象
     */
    UFUNCTION(BlueprintCallable, Category = "Message Bus|Subscription")
    void UnsubscribeAll(UObject* Subscriber);
    
    // ===== 消息历史 =====
    
    /**
     * @brief 获取消息历史
     * @param MessageType 消息类型（无效则返回所有类型）
     * @param MaxCount 最大返回数量
     * @return 消息列表
     */
    UFUNCTION(BlueprintCallable, Category = "Message Bus|History")
    TArray<FSyMessage> GetMessageHistory(FGameplayTag MessageType, int32 MaxCount = 10) const;
    
    /**
     * @brief 清除消息历史
     */
    UFUNCTION(BlueprintCallable, Category = "Message Bus|History")
    void ClearMessageHistory();
    
    /**
     * @brief 设置消息历史大小限制
     * @param MaxSize 每个消息类型保留的最大历史条数
     */
    UFUNCTION(BlueprintCallable, Category = "Message Bus|History")
    void SetHistoryMaxSize(int32 MaxSize);

private:
    // ===== 订阅数据结构 =====
    
    /** 消息订阅表（Filter方式，保持兼容） */
    TMultiMap<USyMessageFilterComposer*, UObject*> MessageSubscriptions;
    
    /** 按消息类型分组的订阅者（新的智能订阅） */
    TMap<FGameplayTag, TArray<TWeakObjectPtr<UObject>>> TypeBasedSubscribers;
    
    // ===== 消息历史 =====
    
    /** 消息历史记录（按类型存储） */
    TMap<FGameplayTag, TArray<FSyMessage>> MessageHistory;
    
    /** 历史记录大小限制 */
    int32 HistoryMaxSize = 50;
    
    // ===== 消息队列（支持优先级） =====
    
    /** 待处理的消息队列 */
    TArray<FSyMessage> MessageQueue;
    
    /** 是否有待处理的消息 */
    bool bHasPendingMessages = false;
    
    // ===== 内部方法 =====
    
    /** 消息过滤和分发 */
    void DispatchMessage(const FSyMessage& Message);
    
    /** 处理消息队列 */
    void ProcessMessageQueue();
    
    /** 添加到历史记录 */
    void AddToHistory(const FSyMessage& Message);
    
    /** 精准广播给订阅者 */
    void BroadcastToTypeSubscribers(const FSyMessage& Message);
    
    /** 清理无效订阅者 */
    void CleanupInvalidSubscribers();
}; 
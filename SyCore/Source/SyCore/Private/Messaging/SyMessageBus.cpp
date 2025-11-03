#include "Messaging/SyMessageBus.h"
#include "Messaging/SyMessageReceiver.h"
#include "Messaging/SyMessageFilter.h"
#include "Foundation/SyLogging.h"
#include "TimerManager.h"
#include "Engine/World.h"

void USyMessageBus::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogSyMessage, Log, TEXT("Message Bus initialized"));
}

void USyMessageBus::Deinitialize()
{
    // 清理所有订阅
    MessageSubscriptions.Empty();
    TypeBasedSubscribers.Empty();
    MessageHistory.Empty();
    MessageQueue.Empty();
    
    UE_LOG(LogSyMessage, Log, TEXT("Message Bus deinitialized"));
    Super::Deinitialize();
}

void USyMessageBus::BroadcastMessage(const FSyMessage& Message)
{
    UE_LOG(LogSyMessage, Verbose, TEXT("📨 Broadcasting message - Type=%s, SourceType=%s, Priority=%s"), 
        *Message.Content.MessageType.ToString(),
        *Message.Source.SourceType.ToString(),
        Message.Priority == ESyMessagePriority::Immediate ? TEXT("Immediate") : TEXT("Queued"));
    
    // 添加到历史
    AddToHistory(Message);
    
    // 根据优先级处理
    if (Message.Priority == ESyMessagePriority::Immediate)
    {
        // 立即分发
        DispatchMessage(Message);
    }
    else
    {
        // 加入队列
        MessageQueue.Add(Message);
        
        // 排序队列（按优先级）
        MessageQueue.Sort([](const FSyMessage& A, const FSyMessage& B)
        {
            return (uint8)A.Priority > (uint8)B.Priority;
        });
        
        // 安排延迟处理
        if (!bHasPendingMessages)
        {
            bHasPendingMessages = true;
            if (UWorld* World = GetWorld())
            {
                World->GetTimerManager().SetTimerForNextTick(this, &USyMessageBus::ProcessMessageQueue);
            }
        }
    }
}

void USyMessageBus::BroadcastMessageWithPriority(FSyMessage Message, ESyMessagePriority Priority)
{
    Message.Priority = Priority;
    BroadcastMessage(Message);
}

void USyMessageBus::SubscribeWithFilter(USyMessageFilterComposer* Filter, UObject* Subscriber)
{
    if (Filter && Subscriber)
    {
        MessageSubscriptions.Add(Filter, Subscriber);
    }
}

void USyMessageBus::UnsubscribeWithFilter(USyMessageFilterComposer* Filter, UObject* Subscriber)
{
    if (Filter && Subscriber)
    {
        MessageSubscriptions.Remove(Filter, Subscriber);
    }
}

TArray<UObject*> USyMessageBus::GetSubscribersForFilter(USyMessageFilterComposer* Filter) const
{
    TArray<UObject*> Subscribers;
    if (Filter)
    {
        MessageSubscriptions.MultiFind(Filter, Subscribers);
    }
    return Subscribers;
}

// ===== 智能订阅实现 =====

void USyMessageBus::SubscribeToMessageType(FGameplayTag MessageType, UObject* Subscriber)
{
    if (!MessageType.IsValid() || !Subscriber)
    {
        UE_LOG(LogSyMessage, Warning, TEXT("SubscribeToMessageType: Invalid MessageType or Subscriber"));
        return;
    }
    
    TArray<TWeakObjectPtr<UObject>>& Subscribers = TypeBasedSubscribers.FindOrAdd(MessageType);
    
    // 检查是否已订阅
    for (const TWeakObjectPtr<UObject>& WeakSub : Subscribers)
    {
        if (WeakSub.Get() == Subscriber)
        {
            UE_LOG(LogSyMessage, Verbose, TEXT("Subscriber %s already subscribed to message type: %s"),
                *Subscriber->GetName(), *MessageType.ToString());
            return;
        }
    }
    
    Subscribers.Add(Subscriber);
    UE_LOG(LogSyMessage, Log, TEXT("✅ Subscriber %s subscribed to message type: %s"),
        *Subscriber->GetName(), *MessageType.ToString());
}

void USyMessageBus::UnsubscribeFromMessageType(FGameplayTag MessageType, UObject* Subscriber)
{
    if (!MessageType.IsValid() || !Subscriber)
    {
        return;
    }
    
    TArray<TWeakObjectPtr<UObject>>* SubscribersPtr = TypeBasedSubscribers.Find(MessageType);
    if (!SubscribersPtr)
    {
        return;
    }
    
    int32 RemovedCount = SubscribersPtr->RemoveAll([Subscriber](const TWeakObjectPtr<UObject>& WeakSub)
    {
        return WeakSub.Get() == Subscriber;
    });
    
    if (RemovedCount > 0)
    {
        UE_LOG(LogSyMessage, Log, TEXT("Unsubscribed %s from message type: %s"),
            *Subscriber->GetName(), *MessageType.ToString());
    }
}

void USyMessageBus::UnsubscribeAll(UObject* Subscriber)
{
    if (!Subscriber)
    {
        return;
    }
    
    int32 TotalRemoved = 0;
    
    for (auto& Pair : TypeBasedSubscribers)
    {
        TotalRemoved += Pair.Value.RemoveAll([Subscriber](const TWeakObjectPtr<UObject>& WeakSub)
        {
            return WeakSub.Get() == Subscriber;
        });
    }
    
    if (TotalRemoved > 0)
    {
        UE_LOG(LogSyMessage, Log, TEXT("Unsubscribed %s from all message types (removed %d subscriptions)"),
            *Subscriber->GetName(), TotalRemoved);
    }
}

// ===== 消息历史实现 =====

TArray<FSyMessage> USyMessageBus::GetMessageHistory(FGameplayTag MessageType, int32 MaxCount) const
{
    TArray<FSyMessage> Result;
    
    if (MessageType.IsValid())
    {
        // 返回特定类型的历史
        if (const TArray<FSyMessage>* HistoryPtr = MessageHistory.Find(MessageType))
        {
            int32 StartIndex = FMath::Max(0, HistoryPtr->Num() - MaxCount);
            for (int32 i = StartIndex; i < HistoryPtr->Num(); ++i)
            {
                Result.Add((*HistoryPtr)[i]);
            }
        }
    }
    else
    {
        // 返回所有类型的历史
        for (const auto& Pair : MessageHistory)
        {
            Result.Append(Pair.Value);
        }
        
        // 排序并限制数量
        Result.Sort([](const FSyMessage& A, const FSyMessage& B)
        {
            return A.Timestamp > B.Timestamp;
        });
        
        if (Result.Num() > MaxCount)
        {
            Result.SetNum(MaxCount);
        }
    }
    
    return Result;
}

void USyMessageBus::ClearMessageHistory()
{
    MessageHistory.Empty();
    UE_LOG(LogSyMessage, Log, TEXT("Message history cleared"));
}

void USyMessageBus::SetHistoryMaxSize(int32 MaxSize)
{
    HistoryMaxSize = FMath::Max(1, MaxSize);
    UE_LOG(LogSyMessage, Log, TEXT("Message history max size set to: %d"), HistoryMaxSize);
}

// ===== 内部方法实现 =====

void USyMessageBus::ProcessMessageQueue()
{
    bHasPendingMessages = false;
    
    // 处理队列中的所有消息
    for (const FSyMessage& Message : MessageQueue)
    {
        DispatchMessage(Message);
    }
    
    MessageQueue.Empty();
}

void USyMessageBus::AddToHistory(const FSyMessage& Message)
{
    if (!Message.Content.MessageType.IsValid())
    {
        return;
    }
    
    TArray<FSyMessage>& History = MessageHistory.FindOrAdd(Message.Content.MessageType);
    History.Add(Message);
    
    // 限制历史大小
    if (History.Num() > HistoryMaxSize)
    {
        History.RemoveAt(0, History.Num() - HistoryMaxSize);
    }
}

void USyMessageBus::BroadcastToTypeSubscribers(const FSyMessage& Message)
{
    if (!Message.Content.MessageType.IsValid())
    {
        return;
    }
    
    TArray<TWeakObjectPtr<UObject>>* SubscribersPtr = TypeBasedSubscribers.Find(Message.Content.MessageType);
    if (!SubscribersPtr || SubscribersPtr->Num() == 0)
    {
        return;
    }
    
    // 清理无效订阅者
    int32 InvalidCount = SubscribersPtr->RemoveAll([](const TWeakObjectPtr<UObject>& WeakSub)
    {
        return !WeakSub.IsValid();
    });
    
    if (InvalidCount > 0)
    {
        UE_LOG(LogSyMessage, Verbose, TEXT("Cleaned up %d invalid subscribers"), InvalidCount);
    }
    
    // 广播给有效订阅者
    int32 BroadcastCount = 0;
    for (const TWeakObjectPtr<UObject>& WeakSub : *SubscribersPtr)
    {
        if (UObject* Subscriber = WeakSub.Get())
        {
            if (Subscriber->Implements<USyMessageReceiver>())
            {
                ISyMessageReceiver::Execute_OnMessageReceived(Subscriber, Message);
                BroadcastCount++;
            }
        }
    }
    
    UE_LOG(LogSyMessage, VeryVerbose, TEXT("📢 Broadcasted to %d subscribers for message type: %s"),
        BroadcastCount, *Message.Content.MessageType.ToString());
}

void USyMessageBus::CleanupInvalidSubscribers()
{
    int32 TotalCleaned = 0;
    
    for (auto& Pair : TypeBasedSubscribers)
    {
        TotalCleaned += Pair.Value.RemoveAll([](const TWeakObjectPtr<UObject>& WeakSub)
        {
            return !WeakSub.IsValid();
        });
    }
    
    if (TotalCleaned > 0)
    {
        UE_LOG(LogSyMessage, Log, TEXT("🧹 Cleaned up %d invalid subscribers"), TotalCleaned);
    }
}

void USyMessageBus::DispatchMessage(const FSyMessage& Message)
{
    // 收集所有匹配的订阅者
    TSet<UObject*> MatchedSubscribers;
    
    // 1. 通过 Filter 匹配（保持兼容）
    for (auto& Subscription : MessageSubscriptions)
    {
        USyMessageFilterComposer* Filter = Subscription.Key;
        
        if (Filter && Filter->Matches(Message))
        {
            TArray<UObject*> FilterSubscribers = GetSubscribersForFilter(Filter);
            MatchedSubscribers.Append(FilterSubscribers);
        }
    }
    
    // 2. 通过智能订阅匹配
    BroadcastToTypeSubscribers(Message);

    UE_LOG(LogSyMessage, VeryVerbose, TEXT("Dispatching message - Filter Matched Subscribers=%d"), 
        MatchedSubscribers.Num());

    // 通知所有匹配的订阅者（Filter方式）
    for (UObject* Subscriber : MatchedSubscribers)
    {
        if (Subscriber && Subscriber->Implements<USyMessageReceiver>())
        {
            ISyMessageReceiver::Execute_OnMessageReceived(Subscriber, Message);
        }
    }
} 
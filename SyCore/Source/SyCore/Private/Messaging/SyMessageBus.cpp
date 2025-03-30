#include "Messaging/SyMessageBus.h"
#include "Messaging/SyMessageReceiver.h"
#include "Messaging/SyMessageFilter.h"

void USyMessageBus::BroadcastMessage(const FSyMessage& Message)
{
    DispatchMessage(Message);
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

void USyMessageBus::DispatchMessage(const FSyMessage& Message)
{
    // 收集所有匹配的订阅者
    TSet<UObject*> MatchedSubscribers;
    
    // 遍历所有过滤器
    for (auto& Subscription : MessageSubscriptions)
    {
        USyMessageFilterComposer* Filter = Subscription.Key;
        
        // 检查过滤器是否匹配
        if (Filter && Filter->Matches(Message))
        {
            // 获取该过滤器的所有订阅者
            TArray<UObject*> FilterSubscribers = GetSubscribersForFilter(Filter);
            MatchedSubscribers.Append(FilterSubscribers);
        }
    }

    // 通知所有匹配的订阅者
    for (UObject* Subscriber : MatchedSubscribers)
    {
        if (Subscriber && Subscriber->Implements<USyMessageReceiver>())
        {
            ISyMessageReceiver::Execute_OnMessageReceived(Subscriber, Message);
        }
    }
} 
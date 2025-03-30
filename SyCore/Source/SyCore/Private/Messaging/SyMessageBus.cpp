#include "Messaging/SyMessageBus.h"
#include "Messaging/SyMessageReceiver.h"

void USyMessageBus::BroadcastMessage(const FSyMessage& Message)
{
    DispatchMessage(Message);
}

void USyMessageBus::SubscribeToMessageType(const FGameplayTag& MessageType, UObject* Subscriber)
{
    if (Subscriber && MessageType.IsValid())
    {
        MessageTypeSubscriptions.Add(MessageType, Subscriber);
    }
}

void USyMessageBus::SubscribeToSourceType(const FGameplayTag& SourceType, UObject* Subscriber)
{
    if (Subscriber && SourceType.IsValid())
    {
        SourceTypeSubscriptions.Add(SourceType, Subscriber);
    }
}

void USyMessageBus::UnsubscribeFromMessageType(const FGameplayTag& MessageType, UObject* Subscriber)
{
    if (Subscriber && MessageType.IsValid())
    {
        MessageTypeSubscriptions.Remove(MessageType, Subscriber);
    }
}

void USyMessageBus::UnsubscribeFromSourceType(const FGameplayTag& SourceType, UObject* Subscriber)
{
    if (Subscriber && SourceType.IsValid())
    {
        SourceTypeSubscriptions.Remove(SourceType, Subscriber);
    }
}

TArray<UObject*> USyMessageBus::GetMessageTypeSubscribers(const FGameplayTag& MessageType) const
{
    TArray<UObject*> Subscribers;
    MessageTypeSubscriptions.MultiFind(MessageType, Subscribers);
    return Subscribers;
}

TArray<UObject*> USyMessageBus::GetSourceTypeSubscribers(const FGameplayTag& SourceType) const
{
    TArray<UObject*> Subscribers;
    SourceTypeSubscriptions.MultiFind(SourceType, Subscribers);
    return Subscribers;
}

void USyMessageBus::DispatchMessage(const FSyMessage& Message)
{
    // 获取消息类型订阅者
    TArray<UObject*> MessageTypeSubscribers = GetMessageTypeSubscribers(Message.Content.MessageType);
    
    // 获取源类型订阅者
    TArray<UObject*> SourceTypeSubscribers = GetSourceTypeSubscribers(Message.Source.SourceType);

    // 合并订阅者列表（去重）
    TSet<UObject*> UniqueSubscribers;
    UniqueSubscribers.Append(MessageTypeSubscribers);
    UniqueSubscribers.Append(SourceTypeSubscribers);

    // 通知所有订阅者
    for (UObject* Subscriber : UniqueSubscribers)
    {
        if (Subscriber && Subscriber->Implements<USyMessageReceiver>())
        {
            ISyMessageReceiver::Execute_OnMessageReceived(Subscriber, Message);
        }
    }
} 
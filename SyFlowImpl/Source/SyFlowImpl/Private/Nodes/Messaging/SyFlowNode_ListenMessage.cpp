#include "Nodes/Messaging/SyFlowNode_ListenMessage.h"

USyFlowNode_ListenMessage::USyFlowNode_ListenMessage(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void USyFlowNode_ListenMessage::ExecuteInput(const FName& PinName)
{
    if(PinName == "Start")
    {
        if(USyMessageBus* MessageBus = GetMessageBus())
        {
            // 同时订阅源类型和消息类型
            MessageBus->SubscribeToSourceType(SourceTag, this);
            MessageBus->SubscribeToMessageType(MessageTypeTag, this);
            SubscribedTags.Add(SourceTag);
            SubscribedTags.Add(MessageTypeTag);
        }
    }
    else if(PinName == "Stop")
    {
        if(USyMessageBus* MessageBus = GetMessageBus())
        {
            // 取消订阅
            MessageBus->UnsubscribeFromSourceType(SourceTag, this);
            MessageBus->UnsubscribeFromMessageType(MessageTypeTag, this);
            SubscribedTags.Remove(SourceTag);
            SubscribedTags.Remove(MessageTypeTag);
        }
    }
}

void USyFlowNode_ListenMessage::HandleMessage(const FSyMessage& Message)
{
    // 设置输出数据Pin的值
    MessageContentPin.Value = Message.Content.Metadata.Contains(TEXT("Content")) 
        ? Message.Content.Metadata[TEXT("Content")] 
        : TEXT("");
    
    // 触发输出流
    TriggerOutput("OnMessage", false);
}

FString USyFlowNode_ListenMessage::GetNodeDescription() const
{
    return FString::Printf(TEXT("Listening for messages from source %s with type %s"), 
        *SourceTag.ToString(), 
        *MessageTypeTag.ToString());
} 
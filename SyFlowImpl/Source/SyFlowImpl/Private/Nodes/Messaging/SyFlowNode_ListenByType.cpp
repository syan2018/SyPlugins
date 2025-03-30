#include "Nodes/Messaging/SyFlowNode_ListenByType.h"

USyFlowNode_ListenByType::USyFlowNode_ListenByType(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void USyFlowNode_ListenByType::ExecuteInput(const FName& PinName)
{
    if(PinName == "Start")
    {
        if(USyMessageBus* MessageBus = GetMessageBus())
        {
            MessageBus->SubscribeToMessageType(MessageTypeTag, this);
            SubscribedTags.Add(MessageTypeTag);
        }
    }
    else if(PinName == "Stop")
    {
        if(USyMessageBus* MessageBus = GetMessageBus())
        {
            MessageBus->UnsubscribeFromMessageType(MessageTypeTag, this);
            SubscribedTags.Remove(MessageTypeTag);
        }
    }
}

void USyFlowNode_ListenByType::HandleMessage(const FSyMessage& Message)
{
    // 设置输出数据Pin的值
    SourceTypePin.Value = Message.Source.SourceType;
    SourceIdPin.Value = Message.Source.SourceId.ToString();
    MessageContentPin.Value = Message.Content.Metadata.Contains(TEXT("Content")) 
        ? Message.Content.Metadata[TEXT("Content")] 
        : TEXT("");
    
    // 触发输出流
    TriggerOutput("OnMessage", false);
}

FString USyFlowNode_ListenByType::GetNodeDescription() const
{
    return FString::Printf(TEXT("Listening for message type: %s"), *MessageTypeTag.ToString());
} 
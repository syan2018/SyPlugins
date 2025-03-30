#include "Nodes/Messaging/SyFlowNode_ListenBySource.h"

USyFlowNode_ListenBySource::USyFlowNode_ListenBySource(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void USyFlowNode_ListenBySource::ExecuteInput(const FName& PinName)
{
    if(PinName == "Start")
    {
        if(USyMessageBus* MessageBus = GetMessageBus())
        {
            MessageBus->SubscribeToSourceType(SourceTag, this);
            SubscribedTags.Add(SourceTag);
        }
    }
    else if(PinName == "Stop")
    {
        if(USyMessageBus* MessageBus = GetMessageBus())
        {
            MessageBus->UnsubscribeFromSourceType(SourceTag, this);
            SubscribedTags.Remove(SourceTag);
        }
    }
}

void USyFlowNode_ListenBySource::HandleMessage(const FSyMessage& Message)
{
    // 设置输出数据Pin的值
    MessageTypePin.Value = Message.Content.MessageType;
    MessageContentPin.Value = Message.Content.Metadata.Contains(TEXT("Content")) 
        ? Message.Content.Metadata[TEXT("Content")] 
        : TEXT("");
    
    // 触发输出流
    TriggerOutput("OnMessage", false);
}

FString USyFlowNode_ListenBySource::GetNodeDescription() const
{
    return FString::Printf(TEXT("Listening for messages from source: %s"), *SourceTag.ToString());
} 
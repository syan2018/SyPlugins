#include "FlowNodes/Messaging/SyFlowNode_ListenByType.h"
#include "Messaging/SyMessageFilter.h"

USyFlowNode_ListenByType::USyFlowNode_ListenByType(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
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

USyMessageFilterComposer* USyFlowNode_ListenByType::CreateMessageFilter() const
{
    USyMessageFilterComposer* Filter = NewObject<USyMessageFilterComposer>();

    // 添加消息类型过滤
    if (MessageType.IsValid())
    {
        USyMessageTypeFilter* TypeFilter = NewObject<USyMessageTypeFilter>();
        TypeFilter->MessageType = MessageType;
        Filter->AddFilter(TypeFilter);
    }

    return Filter;
}

FString USyFlowNode_ListenByType::GetNodeDescription() const
{
    return FString::Printf(TEXT("Listen Message By Type\nType: %s"), *MessageType.ToString());
}

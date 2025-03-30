#include "Nodes/Messaging/SyFlowNode_ListenMessage.h"
#include "Messaging/SyMessageBus.h"
#include "Messaging/SyMessageFilter.h"

USyFlowNode_ListenMessage::USyFlowNode_ListenMessage(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
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

USyMessageFilterComposer* USyFlowNode_ListenMessage::CreateMessageFilter() const
{
    USyMessageFilterComposer* Filter = NewObject<USyMessageFilterComposer>();

    // 添加源类型过滤
    if (SourceTag.IsValid())
    {
        USySourceTypeFilter* TypeFilter = NewObject<USySourceTypeFilter>();
        TypeFilter->SourceType = SourceTag;
        Filter->AddFilter(TypeFilter);
    }

    // 添加GUID过滤
    if (SourceIdentity.IsValid())
    {
        USySourceGuidFilter* GuidFilter = NewObject<USySourceGuidFilter>();
        GuidFilter->SourceGuid = SourceIdentity;
        Filter->AddFilter(GuidFilter);
    }

    // 添加别名过滤
    if (!SourceAlias.IsNone())
    {
        USySourceAliasFilter* AliasFilter = NewObject<USySourceAliasFilter>();
        AliasFilter->SourceAlias = SourceAlias;
        Filter->AddFilter(AliasFilter);
    }

    // 添加消息类型过滤
    if (MessageType.IsValid())
    {
        USyMessageTypeFilter* MessageTypeFilter = NewObject<USyMessageTypeFilter>();
        MessageTypeFilter->MessageType = MessageType;
        Filter->AddFilter(MessageTypeFilter);
    }

    return Filter;
}

FString USyFlowNode_ListenMessage::GetNodeDescription() const
{
    return FString::Printf(TEXT("Listen Message\nSource Type: %s\nMessage Type: %s"),
        *SourceTag.ToString(),
        *MessageType.ToString());
} 
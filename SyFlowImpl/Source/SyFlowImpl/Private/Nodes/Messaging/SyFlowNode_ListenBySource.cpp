#include "Nodes/Messaging/SyFlowNode_ListenBySource.h"
#include "Messaging/SyMessageBus.h"
#include "Messaging/SyMessageFilter.h"

USyFlowNode_ListenBySource::USyFlowNode_ListenBySource(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
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

USyMessageFilterComposer* USyFlowNode_ListenBySource::CreateMessageFilter() const
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

    return Filter;
}

FString USyFlowNode_ListenBySource::GetNodeDescription() const
{
    return FString::Printf(TEXT("Listen Message By Source\nType: %s\nAlias: %s"),
        *SourceTag.ToString(),
        *SourceAlias.ToString());
}

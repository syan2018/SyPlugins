#pragma once

#include "SyFlowNode_MessageBase.h"
#include "SyFlowNode_ListenMessage.generated.h"

UCLASS(NotBlueprintable, meta = (DisplayName = "Listen Message"))
class SYFLOWIMPL_API USyFlowNode_ListenMessage : public USyFlowNode_MessageBase
{
    GENERATED_UCLASS_BODY()

public:
    // 构造函数
    USyFlowNode_ListenMessage();

    // 节点描述
    virtual FString GetNodeDescription() const override;

protected:
    
    // 处理消息
    virtual void HandleMessage(const FSyMessage& Message) override;

    // 创建消息过滤器
    virtual USyMessageFilterComposer* CreateMessageFilter() const override;

    // 过滤配置
    UPROPERTY(EditAnywhere, Category = "Filter|Source", meta = (DisplayName = "Source Type"))
    FGameplayTag SourceTag;

    UPROPERTY(EditAnywhere, Category = "Filter|Source", meta = (DisplayName = "Source Identity"))
    FGuid SourceIdentity;

    UPROPERTY(EditAnywhere, Category = "Filter|Source", meta = (DisplayName = "Source Alias"))
    FName SourceAlias;
    
    UPROPERTY(EditAnywhere, Category = "Filter|Message", meta = (DisplayName = "Message Type"))
    FGameplayTag MessageType;
    
    // 消息内容数据
    UPROPERTY()
    FFlowDataPinOutputProperty_String MessageContentPin;
    
};

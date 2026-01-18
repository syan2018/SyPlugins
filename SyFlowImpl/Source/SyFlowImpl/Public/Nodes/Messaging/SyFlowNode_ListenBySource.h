#pragma once

#include "SyFlowNode_MessageBase.h"
#include "SyFlowNode_ListenBySource.generated.h"

UCLASS(NotBlueprintable, meta = (DisplayName = "Listen Message By Source"))
class SYFLOWIMPL_API USyFlowNode_ListenBySource : public USyFlowNode_MessageBase
{
    GENERATED_UCLASS_BODY()

public:
    // 构造函数
    USyFlowNode_ListenBySource();

    // 节点描述
    virtual FString GetNodeDescription() const override;

protected:

    // 处理消息
    virtual void HandleMessage(const FSyMessage& Message) override;

    // 创建消息过滤器
    virtual USyMessageFilterComposer* CreateMessageFilter() const override;

    // 来源类型配置
    UPROPERTY(EditAnywhere, Category = "Filter|Source", meta = (DisplayName = "Source Type"))
    FGameplayTag SourceTag;

    UPROPERTY(EditAnywhere, Category = "Filter|Source", meta = (DisplayName = "Source Identity"))
    FGuid SourceIdentity;

    UPROPERTY(EditAnywhere, Category = "Filter|Source", meta = (DisplayName = "Source Alias"))
    FName SourceAlias;
    
    // 消息类型数据
    UPROPERTY()
    FFlowDataPinOutputProperty_GameplayTag MessageTypePin;
    
    // 消息内容数据
    UPROPERTY()
    FFlowDataPinOutputProperty_String MessageContentPin;
};

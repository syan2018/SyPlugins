#pragma once

#include "SyFlowNode_MessageBase.h"
#include "SyFlowNode_ListenByType.generated.h"

UCLASS(NotBlueprintable, meta = (DisplayName = "Listen Message By Type"))
class SYFLOWIMPL_API USyFlowNode_ListenByType : public USyFlowNode_MessageBase
{
    GENERATED_UCLASS_BODY()

public:
    // 构造函数
    USyFlowNode_ListenByType();

    // 节点描述
    virtual FString GetNodeDescription() const override;

protected:
    // 执行输入Pin
    virtual void ExecuteInput(const FName& PinName) override;
    
    // 处理消息
    virtual void HandleMessage(const FSyMessage& Message) override;

    // 消息类型配置
    UPROPERTY(EditAnywhere, Category = "Filter|Message", meta = (DisplayName = "Message Type"))
    FGameplayTag MessageTypeTag;

    // 消息来源数据
    UPROPERTY()
    FFlowDataPinOutputProperty_GameplayTag SourceTypePin;
    
    UPROPERTY()
    FFlowDataPinOutputProperty_String SourceIdPin;
    
    // 消息内容数据
    UPROPERTY()
    FFlowDataPinOutputProperty_String MessageContentPin;
}; 
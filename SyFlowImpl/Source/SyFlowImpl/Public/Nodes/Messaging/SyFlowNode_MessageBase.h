#pragma once

#include "Nodes/FlowNode.h"
#include "Messaging/SyMessageBus.h"
#include "Messaging/SyMessageTypes.h"
#include "Messaging/SyMessageFilter.h"
#include "Messaging/SyMessageReceiver.h"
#include "SyFlowNode_MessageBase.generated.h"

UCLASS(Abstract)
class SYFLOWIMPL_API USyFlowNode_MessageBase : public UFlowNode, public ISyMessageReceiver
{
    GENERATED_UCLASS_BODY()

public:
    // 节点分类
    virtual FString GetNodeCategory() const override { return TEXT("SyCore|MessageBus"); }
    
    // 节点描述
    virtual FString GetNodeDescription() const override PURE_VIRTUAL(GetNodeDescription, return TEXT(""););

    // 实现消息接收接口
    virtual void OnMessageReceived_Implementation(const FSyMessage& Message) override;

protected:
    // 基础消息处理函数
    virtual void HandleMessage(const FSyMessage& Message) PURE_VIRTUAL(HandleMessage);
    
    // 获取消息总线
    USyMessageBus* GetMessageBus() const;
    
    // 清理订阅
    virtual void Cleanup() override;

    // 执行输入Pin
    virtual void ExecuteInput(const FName& PinName) override;

    // 创建消息过滤器
    virtual USyMessageFilterComposer* CreateMessageFilter() const PURE_VIRTUAL(CreateMessageFilter, return nullptr;);

    // 订阅状态
    UPROPERTY()
    TArray<FGameplayTag> SubscribedTags;

private:
    // 消息过滤器
    UPROPERTY()
    USyMessageFilterComposer* MessageFilter;
}; 
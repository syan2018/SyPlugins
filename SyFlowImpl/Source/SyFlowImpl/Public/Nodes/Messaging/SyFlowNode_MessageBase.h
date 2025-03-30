#pragma once

#include "Nodes/FlowNode.h"
#include "Messaging/SyMessageBus.h"
#include "Messaging/SyMessageTypes.h"
#include "SyFlowNode_MessageBase.generated.h"

UCLASS(Abstract)
class SYFLOWIMPL_API USyFlowNode_MessageBase : public UFlowNode
{
    GENERATED_UCLASS_BODY()

public:
    // 节点分类
    virtual FString GetNodeCategory() const override { return TEXT("SyCore|MessageBus"); }
    
    // 节点描述
    virtual FString GetNodeDescription() const override PURE_VIRTUAL(GetNodeDescription, return TEXT(""););

protected:
    // 基础消息处理函数
    virtual void HandleMessage(const FSyMessage& Message) PURE_VIRTUAL(HandleMessage);
    
    // 获取消息总线
    USyMessageBus* GetMessageBus() const;
    
    // 清理订阅
    virtual void Cleanup() override;
    
    // 订阅状态
    UPROPERTY()
    TArray<FGameplayTag> SubscribedTags;
}; 
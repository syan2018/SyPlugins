#include "Nodes/Messaging/SyFlowNode_MessageBase.h"
#include "Messaging/SyMessageBus.h"
#include "Foundation/Utilities/SySubsystemUtils.h"
#include "Nodes/FlowPin.h"

USyFlowNode_MessageBase::USyFlowNode_MessageBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // 设置标准的输入输出Pin
    InputPins = {FFlowPin(TEXT("Start")), FFlowPin(TEXT("Stop"))};
    OutputPins = {FFlowPin(TEXT("OnMessage"))};
}

void USyFlowNode_MessageBase::OnMessageReceived_Implementation(const FSyMessage& Message)
{
    // 调用子类的消息处理函数
    HandleMessage(Message);
}

USyMessageBus* USyFlowNode_MessageBase::GetMessageBus() const
{
    return GetWorld()->GetGameInstance()->GetSubsystem<USyMessageBus>();
}

void USyFlowNode_MessageBase::Cleanup()
{
    if (MessageFilter)
    {
        if (USyMessageBus* MessageBus = GetMessageBus())
        {
            MessageBus->UnsubscribeWithFilter(MessageFilter, this);
        }
        MessageFilter = nullptr;
    }
}

void USyFlowNode_MessageBase::ExecuteInput(const FName& PinName)
{
    if (PinName == "Start")
    {
        if (!MessageFilter)
        {
            MessageFilter = CreateMessageFilter();
        }

        if (MessageFilter && GetMessageBus())
        {
            GetMessageBus()->SubscribeWithFilter(MessageFilter, this);
        }
    }
    else if (PinName == "Stop")
    {
        Cleanup();
    }
}

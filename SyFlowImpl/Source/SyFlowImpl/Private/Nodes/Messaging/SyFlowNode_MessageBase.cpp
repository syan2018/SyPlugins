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

USyMessageBus* USyFlowNode_MessageBase::GetMessageBus() const
{
    return SySubsystemUtils::GetSubsystem<USyMessageBus>(GetWorld());
}

void USyFlowNode_MessageBase::Cleanup()
{
    // 清理所有订阅
    if (USyMessageBus* MessageBus = GetMessageBus())
    {
        for (const FGameplayTag& Tag : SubscribedTags)
        {
            MessageBus->UnsubscribeFromSourceType(Tag, this);
        }
        SubscribedTags.Empty();
    }
    
    Super::Cleanup();
}

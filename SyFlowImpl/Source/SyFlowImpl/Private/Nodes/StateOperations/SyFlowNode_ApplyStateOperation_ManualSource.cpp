#include "Nodes/StateOperations/SyFlowNode_ApplyStateOperation_ManualSource.h"

FSyOperationSource USyFlowNode_ApplyStateOperation_ManualSource::GetOperationSource() const
{
    // Return the source configured directly in the details panel.
    return ManualOperationSource;
}

#if WITH_EDITOR
FText USyFlowNode_ApplyStateOperation_ManualSource::GetNodeToolTip() const
{
    return FText::FromString(TEXT("Applies a state operation using a manually configured Source."));
}
#endif 
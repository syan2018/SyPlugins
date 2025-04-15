#pragma once

#include "Nodes/StateOperations/SyFlowNode_StateOperationBase.h"
#include "SyFlowNode_ApplyStateOperation_ManualSource.generated.h"

/**
 * Applies a state operation where the Source is manually configured in the node details panel.
 */
UCLASS(DisplayName = "Apply State Operation (Manual Source)")
class SYFLOWIMPL_API USyFlowNode_ApplyStateOperation_ManualSource : public USyFlowNode_StateOperationBase
{
    GENERATED_BODY()

protected:
    /** Manually configure the source of the operation here. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Operation")
    FSyOperationSource ManualOperationSource;

    // --- Overrides --- 
    virtual FSyOperationSource GetOperationSource() const override;

public:
    // --- Node Info --- 
#if WITH_EDITOR
    virtual FText GetNodeTitle() const override { return FText::FromString("Apply State Op (Manual Source)"); }
    virtual FText GetNodeToolTip() const override;
#endif
}; 
#pragma once

#include "Nodes/StateOperations/SyFlowNode_StateOperationBase.h"
#include "SyFlowNode_ApplyStateOperation_Simple.generated.h"

/**
 * Applies a state operation with a single Modifier and a manually configured Source.
 * This is a simple example for applying basic state changes.
 */
UCLASS(DisplayName = "Apply State Operation (Simple)")
class SYFLOWIMPL_API USyFlowNode_ApplyStateOperation_Simple : public USyFlowNode_StateOperationBase
{
    GENERATED_BODY()

protected:
    /** Manually configure the source of the operation here. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Operation")
    FSyOperationSource ManualOperationSource;

    /** The specific state tag within the target to modify. Only used by simple derived nodes like ApplyStateOperation_Simple. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Operation", meta = (DisplayName = "Modifier (Single Param)"))
    FSyStateParams OpModifier;
    
    // --- Overrides --- 
    virtual FSyOperationSource GetOperationSource() const override;

    virtual FSyOperationModifier GetOperationModifier() const override;

public:
    // --- Node Info --- 
#if WITH_EDITOR
    virtual FText GetNodeTitle() const override { return FText::FromString("Apply State Op (Simple)"); }
    virtual FText GetNodeToolTip() const override { return FText::FromString("Applies a state operation with one Modifier (from base class) and a manually configured Source."); }
#endif
};

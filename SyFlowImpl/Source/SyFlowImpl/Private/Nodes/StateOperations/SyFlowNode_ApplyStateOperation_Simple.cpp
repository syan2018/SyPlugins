// Copyright Epic Games, Inc. All Rights Reserved.

#include "Nodes/StateOperations/SyFlowNode_ApplyStateOperation_Simple.h"
#include "Operations/OperationTypes.h"

FSyOperationSource USyFlowNode_ApplyStateOperation_Simple::GetOperationSource() const
{
    // Return the source configured directly in the details panel.
    return ManualOperationSource;
}

FSyOperationModifier USyFlowNode_ApplyStateOperation_Simple::GetOperationModifier() const
{
    FSyOperationModifier Modifier;
    
    if (OpModifier.Tag.IsValid() && !OpModifier.Params.IsEmpty())
    {
        Modifier.StateModifications.AddStateParams(OpModifier.Tag,OpModifier.Params);
    }
    return Modifier;
}

#if WITH_EDITOR
// Removed tooltip implementation as it's now inline in the header
// FText USyFlowNode_ApplyStateOperation_Simple::GetNodeToolTip() const
// {
//     return FText::FromString("Applies a state operation with one Modifier (from base class) and a manually configured Source.");
// }
#endif 
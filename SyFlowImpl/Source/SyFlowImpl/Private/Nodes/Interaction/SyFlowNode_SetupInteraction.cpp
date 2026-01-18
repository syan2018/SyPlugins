#include "Nodes/Interaction/SyFlowNode_SetupInteraction.h"

#include "FlowLogChannels.h"
#include "Operations/OperationTypes.h" // For Modifier, Source, Params
#include "Core/Metadatas/BasicMetadataValueTypes.h" // For FSyBoolValue
#include "Metadatas/SyGameplayInteractValueTypes.h" // For FSyInteractionListValue (from SyGameplay)

FSyOperationSource USyFlowNode_SetupInteraction::GetOperationSource() const
{
    return ManualOperationSource;
}

FSyOperationModifier USyFlowNode_SetupInteraction::GetOperationModifier() const
{
    FSyOperationModifier Modifier;

    // 1. Add Interactable = true parameter
    if (InteractableTag.IsValid())
    {
        FInstancedStruct InteractableParamStruct = FInstancedStruct::Make(FSyBoolValue(true));
        if(InteractableParamStruct.IsValid())
        {
             Modifier.StateModifications.AddStateParam(InteractableTag, InteractableParamStruct);
        }
        else
        {
            UE_LOG(LogFlow, Warning, TEXT("SetupInteraction Node: Failed to create InstancedStruct for InteractableTag %s"), *InteractableTag.ToString());
        }
    }
    else
    {
         UE_LOG(LogFlow, Warning, TEXT("SetupInteraction Node: InteractableTag is invalid."));
    }

    // 2. Add Interaction Info List parameter
    if (InteractInfoTag.IsValid() && InteractionItem.IsValid())
    {
        // Wrap the configured FSyInteractionListValue in an FInstancedStruct

        FSyInteractionListValue InteractionListValue;
        InteractionListValue.InteractionItems.Add(InteractionItem);
        
        FInstancedStruct InteractInfoParamStruct = FInstancedStruct::Make(InteractionListValue);
        if(InteractInfoParamStruct.IsValid())
        {
            Modifier.StateModifications.AddStateParam(InteractInfoTag, InteractInfoParamStruct);
        }
        else
        {
             UE_LOG(LogFlow, Warning, TEXT("SetupInteraction Node: Failed to create InstancedStruct for InteractInfoTag %s"), *InteractInfoTag.ToString());
        }
    }
     else
    {
         UE_LOG(LogFlow, Warning, TEXT("SetupInteraction Node: InteractInfoTag is invalid."));
    }

    return Modifier;
}

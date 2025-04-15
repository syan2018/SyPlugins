#include "Nodes/StateOperations/SyFlowNode_StateOperationBase.h"

#include "FlowSubsystem.h"
#include "SyStateManager/Public/SyStateManagerSubsystem.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogSyFlowNodeStateOpBase, Log, All);

FSyOperationModifier USyFlowNode_StateOperationBase::GetOperationModifier() const
{
    FSyOperationModifier Modifier;
    if (OpModifier.Tag.IsValid())
    {
        Modifier.AddStateModifications(OpModifier);
    }
    else
    {
        UE_LOG(LogSyFlowNodeStateOpBase, Warning, TEXT("%s: Invalid Modifier StateTag or ParameterData, Modifier will be empty."), *GetNodeTitle().ToString());
    }
    return Modifier;
}

void USyFlowNode_StateOperationBase::ExecuteInput(const FName& PinName)
{
    USyStateManagerSubsystem* StateManager = GetStateManagerSubsystem();
    if (!StateManager)
    {
        UE_LOG(LogSyFlowNodeStateOpBase, Error, TEXT("%s: Could not get StateManagerSubsystem."), *GetNodeTitle().ToString());
        // Optionally trigger an error output pin here
        return;
    }

    // Construct the Modifier
    FSyOperationModifier Modifier = GetOperationModifier();
    
    // Get the Source from the derived class
    FSyOperationSource Source = GetOperationSource();

    // Construct the full Operation
    FSyOperation Operation(Source, Modifier, OperationTarget);

    // Record the operation with the State Manager
    StateManager->RecordOperation(Operation);
    
    UE_LOG(LogSyFlowNodeStateOpBase, Log, TEXT("%s: Recording state operation (OpID: %s) for TargetTag: %s"), 
        *GetNodeTitle().ToString(), *Operation.OperationId.ToString(), *Operation.Target.TargetTypeTag.ToString());

    // Trigger the output pin (assuming a single output pin named "Out")
    TriggerOutput(TEXT("Out"), true);
}

USyStateManagerSubsystem* USyFlowNode_StateOperationBase::GetStateManagerSubsystem() const
{
    if (const UFlowSubsystem* FlowSubsystem = GetFlowSubsystem())
    {
        if (UGameInstance* GameInstance = FlowSubsystem->GetGameInstance())
        {
            return GameInstance->GetSubsystem<USyStateManagerSubsystem>();
        }
    }
    // Fallback if FlowSubsystem is not available (e.g., during editor)
    if (const UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
             return GameInstance->GetSubsystem<USyStateManagerSubsystem>();
        }
    }
    return nullptr;
}

#if WITH_EDITOR
FText USyFlowNode_StateOperationBase::GetNodeToolTip() const
{
    return FText::FromString(TEXT("Base node for applying state modifications. Configure Target and Modifier, derive for Source."));
}
#endif 
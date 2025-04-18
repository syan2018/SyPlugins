#include "Nodes/StateOperations/SyFlowNode_StateOperationBase.h"

#include "FlowSubsystem.h"
#include "SyStateManager/Public/SyStateManagerSubsystem.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"
#include "Engine/GameInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogSyFlowNodeStateOpBase, Log, All);

USyFlowNode_StateOperationBase::USyFlowNode_StateOperationBase()
{
    // Add the new "Unload" input pin alongside the standard "In"
    InputPins = { SyFlowNodeStateOpInputPins::In, SyFlowNodeStateOpInputPins::Unload }; 
    OutputPins = { TEXT("Out") }; // Standard output pin
}



void USyFlowNode_StateOperationBase::ExecuteInput(const FName& PinName)
{
    // Handle the Unload pin first
    if (PinName == SyFlowNodeStateOpInputPins::Unload)
    {   
        ExecuteUnloadInput(PinName); // Call the virtual function for unload logic
        // Note: We might not trigger an output pin for Unload, or add a specific "Unloaded" pin.
        // For now, just execute the unload logic.
        return; 
    }
    
    // Handle the standard "In" pin for applying the operation
    if (PinName == SyFlowNodeStateOpInputPins::In)
    {   
        USyStateManagerSubsystem* StateManager = GetStateManagerSubsystem();
        if (!StateManager)
        {
            UE_LOG(LogSyFlowNodeStateOpBase, Error, TEXT("%s: Could not get StateManagerSubsystem."), *GetNodeTitle().ToString());
            TriggerOutput(TEXT("Out"), true); // Still trigger output?
            return;
        }

        // Construct the operation
        FSyOperation Operation;
        Operation.OperationId = FGuid::NewGuid(); // Generate a unique ID for this operation instance
        Operation.Target = OperationTarget;
        Operation.Source = GetOperationSource(); // Get source from derived class
        Operation.Modifier = GetOperationModifier(); // Get modifier from derived class (or this base class)

        // Record the operation
        bool bSuccess = StateManager->RecordOperation(Operation);
        if (bSuccess)
        {   
            // TODO: Store Operation.OperationId if StateManager confirms?
            // AppliedOperationId = Operation.OperationId; 
            UE_LOG(LogSyFlowNodeStateOpBase, Log, TEXT("%s: Recorded operation %s."), *GetNodeTitle().ToString(), *Operation.OperationId.ToString());
        }
        else
        {
            UE_LOG(LogSyFlowNodeStateOpBase, Warning, TEXT("%s: Failed to record operation %s."), *GetNodeTitle().ToString(), *Operation.OperationId.ToString());
        }

        // Trigger the output pin
        TriggerOutput(TEXT("Out"), true);
    }
}

void USyFlowNode_StateOperationBase::ExecuteUnloadInput(FName PinName)
{
    // Base implementation - does nothing except log a warning.
    // Derived classes could override this to construct and record an "Unload" operation.
    // This requires a corresponding mechanism in the StateManager (e.g., RecordUnloadOperation(FGuid OperationId)).
    UE_LOG(LogSyFlowNodeStateOpBase, Warning, TEXT("%s: 'Unload' pin executed, but unload logic is not implemented in this node or base class. StateManager functionality is required."), *GetNodeTitle().ToString());
    
    // Optionally trigger an output pin like "Unloaded" if added.
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
    return FText::FromString(TEXT("Base node for applying state operations via the State Manager. Configure Target and potentially a single Modifier. Derived nodes define Source and specific Modifiers. Includes an 'Unload' input pin."));
}
#endif 
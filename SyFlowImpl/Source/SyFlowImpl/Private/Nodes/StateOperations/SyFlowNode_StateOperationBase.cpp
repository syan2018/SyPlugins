#include "Nodes/StateOperations/SyFlowNode_StateOperationBase.h"

#include "FlowSubsystem.h"
#include "Manager/SyStateManagerSubsystem.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"
#include "Engine/GameInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogSyFlowNodeStateOpBase, Log, All);

USyFlowNode_StateOperationBase::USyFlowNode_StateOperationBase()
{
    // Add the new "Unload" input pin alongside the standard "In"
    InputPins = { SyFlowNodeStateOpInputPins::In, SyFlowNodeStateOpInputPins::Unload }; 
    // Add the optional Unloaded output pin
    OutputPins = { SyFlowNodeStateOpOutputPins::Out, SyFlowNodeStateOpOutputPins::Unloaded }; 
    AppliedOperationId.Invalidate(); // Ensure it starts invalid
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
        // If an operation was previously applied by this node instance, unload it first
        // This prevents duplicate operations if the node is re-triggered without being unloaded.
        if(AppliedOperationId.IsValid())
        {
            UE_LOG(LogSyFlowNodeStateOpBase, Log, TEXT("%s: Node re-triggered with a valid AppliedOperationId (%s). Attempting to unload previous operation first."), 
                *GetNodeTitle().ToString(), *AppliedOperationId.ToString());
            ExecuteUnloadInput(SyFlowNodeStateOpInputPins::Unload); // Call unload logic internally
            // Note: ExecuteUnloadInput might invalidate AppliedOperationId
        }
        
        USyStateManagerSubsystem* StateManager = GetStateManagerSubsystem();
        if (!StateManager)
        {
            UE_LOG(LogSyFlowNodeStateOpBase, Error, TEXT("%s: Could not get StateManagerSubsystem."), *GetNodeTitle().ToString());
            TriggerOutput(SyFlowNodeStateOpOutputPins::Out, true); 
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
            // Store the ID of the successfully recorded operation
            AppliedOperationId = Operation.OperationId; 
            UE_LOG(LogSyFlowNodeStateOpBase, Log, TEXT("%s: Recorded operation %s."), *GetNodeTitle().ToString(), *AppliedOperationId.ToString());
        }
        else
        {   
            // Ensure ID is invalid if recording failed
            AppliedOperationId.Invalidate();
            UE_LOG(LogSyFlowNodeStateOpBase, Warning, TEXT("%s: Failed to record operation. AppliedOperationId invalidated."), *GetNodeTitle().ToString());
        }

        // Trigger the output pin
        TriggerOutput(SyFlowNodeStateOpOutputPins::Out, true);
    }
}

void USyFlowNode_StateOperationBase::ExecuteUnloadInput(FName PinName)
{
    if (!AppliedOperationId.IsValid())
    {
        UE_LOG(LogSyFlowNodeStateOpBase, Warning, TEXT("%s: 'Unload' triggered, but AppliedOperationId is not valid (already unloaded or never applied?)."), *GetNodeTitle().ToString());
        // Optionally trigger Unloaded pin even if ID was invalid?
        // TriggerOutput(SyFlowNodeStateOpOutputPins::Unloaded, true);
        return; 
    }

    USyStateManagerSubsystem* StateManager = GetStateManagerSubsystem();
    if (!StateManager)
    {
        UE_LOG(LogSyFlowNodeStateOpBase, Error, TEXT("%s: Could not get StateManagerSubsystem during Unload."), *GetNodeTitle().ToString());
        // Optionally trigger Unloaded pin even if StateManager is missing?
        // TriggerOutput(SyFlowNodeStateOpOutputPins::Unloaded, true);
        return;
    }

    FGuid IdToUnload = AppliedOperationId; // Copy before invalidating
    bool bUnloadSuccess = StateManager->UnloadOperation(IdToUnload);

    if (bUnloadSuccess)
    {
        UE_LOG(LogSyFlowNodeStateOpBase, Log, TEXT("%s: Successfully requested unload for operation %s."), *GetNodeTitle().ToString(), *IdToUnload.ToString());
        // Invalidate the stored ID after successful unload request
        AppliedOperationId.Invalidate(); 
        TriggerOutput(SyFlowNodeStateOpOutputPins::Unloaded, true); // Trigger the "Unloaded" pin
    }
    else
    {
        UE_LOG(LogSyFlowNodeStateOpBase, Warning, TEXT("%s: StateManager->UnloadOperation failed for ID %s (Operation might have been unloaded elsewhere?). AppliedOperationId kept intact for potential retry? Decide based on desired behavior."), 
            *GetNodeTitle().ToString(), *IdToUnload.ToString());
        // Decide if we should invalidate the ID even if unload failed in StateManager
        // AppliedOperationId.Invalidate(); 
        // Optionally trigger Unloaded pin even on failure?
        // TriggerOutput(SyFlowNodeStateOpOutputPins::Unloaded, true);
    }
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
    return FText::FromString(TEXT("Base node for applying state operations via the State Manager. Configure Target. Derived nodes define Source and Modifiers. Includes 'Unload' pin to remove the applied operation record using its stored ID."));
}

void USyFlowNode_StateOperationBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // If any property related to the operation configuration changes, invalidate the stored ID
    // This prevents unloading an operation that no longer matches the node's configuration.
    // Check PropertyChangedEvent.GetPropertyName() if more fine-grained control is needed.
    if (PropertyChangedEvent.Property != nullptr)
    {
        AppliedOperationId.Invalidate();
        UE_LOG(LogSyFlowNodeStateOpBase, Log, TEXT("%s: Node configuration changed in editor, invalidated AppliedOperationId."), *GetNodeTitle().ToString());
    }
}
#endif 
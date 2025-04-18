#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "OperationTypes.h" // For FSyOperation types
#include "Nodes/FlowNode.h"
#include "StructUtils/InstancedStruct.h"
#include "SyFlowNode_StateOperationBase.generated.h"

// Forward declarations
class USyStateManagerSubsystem;

// Input Pins
namespace SyFlowNodeStateOpInputPins
{
    const FName In(TEXT("In"));
    const FName Unload(TEXT("Unload")); // New Unload Pin
}

/**
 * Base class for Flow Nodes that apply a state operation via the StateManager.
 * Handles the configuration of the Target and a single Modifier parameter.
 * Derived classes must implement how the Source is determined.
 * Includes an optional "Unload" input to potentially reverse the operation (requires StateManager support).
 */
UCLASS(Abstract, NotBlueprintable)
class SYFLOWIMPL_API USyFlowNode_StateOperationBase : public UFlowNode
{
    GENERATED_BODY()

public: // Public for construction in derived classes
    USyFlowNode_StateOperationBase();

protected:
    // --- Configuration --- 

    /** Defines the target of the operation. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Operation")
    FSyOperationTarget OperationTarget;
    
    // --- Execution Logic --- 

    /** 
     * Derived classes must override this to provide the source of the operation.
     */
    virtual FSyOperationSource GetOperationSource() const PURE_VIRTUAL(USyFlowNode_StateOperationBase::GetOperationSource, return FSyOperationSource(););

    /** 
     * Derived classes should override this to provide the specific modifier parameters for the operation.
     * The base implementation uses the single OpModifier property.
     */
    virtual FSyOperationModifier GetOperationModifier() const PURE_VIRTUAL(USyFlowNode_StateOperationBase::GetOperationModifier, return FSyOperationModifier(););

    /** 
     * Executes the state operation by recording it with the StateManager (when "In" pin is triggered).
     * Also handles the "Unload" pin trigger.
     */
    virtual void ExecuteInput(const FName& PinName) override;

    /** 
     * Executes the logic for unloading/reversing the state operation (when "Unload" pin is triggered).
     * Base implementation logs a warning. Derived classes can override.
     * Requires corresponding functionality in the StateManagerSubsystem.
     */
    virtual void ExecuteUnloadInput(FName PinName);

    /** Helper function to get the State Manager Subsystem. */
    USyStateManagerSubsystem* GetStateManagerSubsystem() const;

    // --- TODO: Store Operation ID? ---
    // UPROPERTY()
    // FGuid AppliedOperationId; // Could store the ID if StateManager returns one upon recording

public:
    // --- Node Info --- 
#if WITH_EDITOR
    virtual FText GetNodeTitle() const override { return FText::FromString("State Op Base"); } 
    virtual FText GetNodeToolTip() const override;
    virtual FString GetNodeCategory() const override { return TEXT("SyPlugin|StateOp"); }
#endif
}; 
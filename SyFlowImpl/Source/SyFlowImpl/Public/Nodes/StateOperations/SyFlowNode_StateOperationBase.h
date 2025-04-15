#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "OperationTypes.h" // For FSyOperation types
#include "Nodes/FlowNode.h"
#include "StructUtils/InstancedStruct.h"
#include "SyFlowNode_StateOperationBase.generated.h"

// Forward declarations
class USyStateManagerSubsystem;

/**
 * Base class for Flow Nodes that apply a state operation via the StateManager.
 * Handles the configuration of the Target and a single Modifier parameter.
 * Derived classes must implement how the Source is determined.
 */
UCLASS(Abstract, NotBlueprintable)
class SYFLOWIMPL_API USyFlowNode_StateOperationBase : public UFlowNode
{
    GENERATED_BODY()

protected:
    // --- Configuration --- 

    /** Defines the target of the operation. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Operation")
    FSyOperationTarget OperationTarget;

    /** The specific state tag within the target to modify. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Operation", meta = (DisplayName = "Modifier"))
    FSyStateParams OpModifier;

    // --- Execution Logic --- 

    /** 
     * Derived classes must override this to provide the source of the operation.
     */
    virtual FSyOperationSource GetOperationSource() const PURE_VIRTUAL(USyFlowNode_StateOperationBase::GetOperationSource, return FSyOperationSource(););

    /** 
     * Derived classes must override this to provide the source of the operation.
     */
    virtual FSyOperationModifier GetOperationModifier() const;

    /** 
     * Executes the state operation by recording it with the StateManager.
     */
    virtual void ExecuteInput(const FName& PinName) override;

    /** Helper function to get the State Manager Subsystem. */
    USyStateManagerSubsystem* GetStateManagerSubsystem() const;

public:
    // --- Node Info --- 
#if WITH_EDITOR
    virtual FText GetNodeTitle() const override { return FText::FromString("State Op Base"); } 
    virtual FText GetNodeToolTip() const override;
    virtual FString GetNodeCategory() const override { return TEXT("SyPlugin|StateOp"); }
#endif
}; 
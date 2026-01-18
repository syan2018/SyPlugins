#pragma once

#include "FlowNodes/StateOperations/SyFlowNode_StateOperationBase.h"
#include "GameplayTagContainer.h" // For FGameplayTag
#include "SyGameplayInteractValueTypes.h"
#include "Metadatas/SyGameplayInteractValueTypes.h" // For FSyInteractionListValue
#include "SyFlowNode_SetupInteraction.generated.h"

/**
 * Applies a state operation specifically designed to set up interaction states.
 * It enables interaction (State.Interact.Interactable = true) and sets 
 * the interaction info list (State.Interact.InteractInfo = InteractionListValue).
 */
UCLASS(DisplayName = "Setup Interaction State")
class SYGAMEPLAY_API USyFlowNode_SetupInteraction : public USyFlowNode_StateOperationBase
{
    GENERATED_BODY()

protected:
    /** Manually configure the source of the operation here. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Operation")
    FSyOperationSource ManualOperationSource;

    /** 指定交互内容，以结构体展示交互类型，常用Flow */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Operation|Interaction Setup", meta = (BaseStruct="/Script/SyGameplay.FSyInteractInfoBase", ExcludeBaseStruct, DisplayName="Interaction Info List"))
    FInstancedStruct InteractionItem;

    /** The tag used for the boolean interactable state. */
    FGameplayTag InteractableTag = FGameplayTag::RequestGameplayTag(FName("State.Interact.Interactable"));

    /** The tag used for storing the interaction info list. */
    FGameplayTag InteractInfoTag = FGameplayTag::RequestGameplayTag(FName("State.Interact.InteractInfo"));

    // --- Overrides --- 
    virtual FSyOperationSource GetOperationSource() const override;
    virtual FSyOperationModifier GetOperationModifier() const override;

public:
    // --- Node Info --- 
#if WITH_EDITOR
    virtual FText GetNodeTitle() const override { return FText::FromString("Setup Interaction State"); }
    virtual FText GetNodeToolTip() const override { return FText::FromString("Applies an operation to enable interaction and set the interaction info list."); }
    virtual FString GetNodeCategory() const override { return TEXT("SyPlugin|Interaction"); }
#endif
};

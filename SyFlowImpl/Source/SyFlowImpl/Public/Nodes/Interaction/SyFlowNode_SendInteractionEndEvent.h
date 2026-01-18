#pragma once

#include "CoreMinimal.h"
#include "Nodes/FlowNode.h"
#include "GameplayTagContainer.h" // For FGameplayTag
#include "SyFlowNode_SendInteractionEndEvent.generated.h"

/**
 * Sends an Event.Interaction.End message via the MessageBus.
 * The sender of the message is the Owner Actor of the Flow Component.
 */
UCLASS(DisplayName = "Send Interaction End Event")
class SYFLOWIMPL_API USyFlowNode_SendInteractionEndEvent : public UFlowNode
{
	GENERATED_BODY()

public:
	USyFlowNode_SendInteractionEndEvent();

protected:
	// --- FlowNode Overrides --- 
	virtual void ExecuteInput(const FName& PinName) override;

#if WITH_EDITOR
	// --- Node Info --- 
	virtual FText GetNodeTitle() const override { return FText::FromString("Send Interact End"); }
	virtual FString GetNodeCategory() const override { return TEXT("SyPlugin|Interaction"); }
	virtual FText GetNodeToolTip() const override { return FText::FromString("Sends Event.Interaction.End message with the Flow Owner as the sender."); }
#endif

	/** The tag for the message to send. */
	UPROPERTY(EditAnywhere, Category = "Message")
	FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Event.Interaction.End"));
};

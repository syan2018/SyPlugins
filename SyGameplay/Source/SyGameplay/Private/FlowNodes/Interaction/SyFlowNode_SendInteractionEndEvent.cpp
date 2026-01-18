#include "FlowNodes/Interaction/SyFlowNode_SendInteractionEndEvent.h"
#include "Nodes/FlowNodeBase.h" // Base class for TryGetRootFlowActorOwner
#include "FlowLogChannels.h" // For logging
#include "GameFramework/Actor.h" // For AActor
#include "Engine/World.h" // For GetWorld()
#include "Engine/GameInstance.h" // For GetGameInstance()
#include "Entity/SyEntityComponent.h"

USyFlowNode_SendInteractionEndEvent::USyFlowNode_SendInteractionEndEvent()
{
	InputPins = { FFlowPin(TEXT("Send")) };
	OutputPins = { FFlowPin(TEXT("Sent")) };
}

void USyFlowNode_SendInteractionEndEvent::ExecuteInput(const FName& PinName)
{
	if (PinName == TEXT("Send"))
	{
		AActor* OwnerActor = TryGetRootFlowActorOwner(); 
		if (!OwnerActor)
		{
			UObject* OwnerObject = TryGetRootFlowObjectOwner();
			UE_LOG(LogFlow, Warning, TEXT("SendInteractionEndEvent: Could not find an Owning Actor via TryGetRootFlowActorOwner(). Owning Object is: %s. Cannot send message via EntityComponent."), *GetNameSafe(OwnerObject));
			TriggerOutput(TEXT("Sent"), true); 
			return;
		}

		// Find the SyEntityComponent on the owner actor
		USyEntityComponent* EntityComponent = OwnerActor->FindComponentByClass<USyEntityComponent>();
		if (!EntityComponent)
		{
			UE_LOG(LogFlow, Warning, TEXT("SendInteractionEndEvent: Could not find SyEntityComponent on Owner Actor %s. Cannot send message."), *OwnerActor->GetName());
			TriggerOutput(TEXT("Sent"), true);
			return;
		}

		// Send the message via the EntityComponent
		UE_LOG(LogFlow, Log, TEXT("Sending message %s via EntityComponent on %s"), *MessageTag.ToString(), *OwnerActor->GetName());
		bool bSent = EntityComponent->BroadcastEvent(MessageTag); 
		if (!bSent)
		{
			UE_LOG(LogFlow, Warning, TEXT("SendInteractionEndEvent: EntityComponent->SendMessage failed for tag %s on actor %s."), *MessageTag.ToString(), *OwnerActor->GetName());
		}
		
		// Trigger the output pin regardless of send success?
		TriggerOutput(TEXT("Sent"), true);
	}
}

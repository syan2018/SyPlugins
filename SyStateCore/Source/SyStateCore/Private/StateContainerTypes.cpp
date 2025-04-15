// Copyright Epic Games, Inc. All Rights Reserved.

#include "StateContainerTypes.h"
#include "StateParameterTypes.h"
#include "StateMetadataTypes.h"
#include "DS_TagMetadata.h"
#include "Logging/LogMacros.h"

// --- Helper Function --- 

/**
 * Finds an existing instance of a specific metadata type within the local storage for a tag, 
 * or adds the provided template instance if not found.
 *
 * @param LocalMetadataArray The FSyStateMetadatas storage for the specific tag.
 * @param TemplateMetadata The template instance (typically from UDS_TagMetadata) representing the desired type.
 * @param StateTag The state tag this instance belongs to (used if adding).
 * @return Pointer to the found or newly added USyStateMetadataBase instance, or nullptr if TemplateMetadata was invalid.
 */
static USyStateMetadataBase* FindOrAddLocalMetadataInstance(FSyStateMetadatas& LocalMetadataArray, USyStateMetadataBase* TemplateMetadata, const FGameplayTag& StateTag)
{
    if (!TemplateMetadata) return nullptr;

    // Try to find existing local instance by type
    for (TObjectPtr<UO_TagMetadata>& LocalInstancePtr : LocalMetadataArray.MetadataArray)
    {
        USyStateMetadataBase* LocalInstance = Cast<USyStateMetadataBase>(LocalInstancePtr);
        if (LocalInstance && LocalInstance->GetClass() == TemplateMetadata->GetClass())
        {
            return LocalInstance; // Found existing
        }
    }

    // Not found, add the template instance 
    // WARNING: Assumes UDS_TagMetadata provides instances safe to add directly.
    // Consider DuplicateObject if UDS returns shared instances that need unique state.
    LocalMetadataArray.AddMetadata(TemplateMetadata);
    TemplateMetadata->SetStateTag(StateTag); // Ensure tag is set on the added instance
    return TemplateMetadata; // Return the newly added instance
}


// --- FSyStateCategories Implementation --- 

void FSyStateCategories::ApplyInitData(const FSyStateParameterSet& InitData)
{
    // Iterate through all provided initialization entries (Tag + Params)
    for (const FSyStateParams& ParamsEntry : InitData.Parameters)
    {
        const FGameplayTag& StateTag = ParamsEntry.Tag;
        const TArray<FInstancedStruct>& InitParams = ParamsEntry.Params; // Provided init parameters for this tag

        // Get metadata instances defined for this tag by the system (UDS)
        TArray<UO_TagMetadata*> MetadataInstances = UDS_TagMetadata::GetTagMetadata(StateTag);

        // Get or create local storage for this tag
        FSyStateMetadatas& LocalMetadataArray = StateData.FindOrAdd(StateTag);

        // Iterate through each metadata type defined for the tag
        for (UO_TagMetadata* MetadataInstanceTemplate : MetadataInstances)
        {
            USyStateMetadataBase* TemplateMetadata = Cast<USyStateMetadataBase>(MetadataInstanceTemplate);
            if (!TemplateMetadata) continue;

            // Find or add the corresponding local instance
            USyStateMetadataBase* TargetMetadata = FindOrAddLocalMetadataInstance(LocalMetadataArray, TemplateMetadata, StateTag);
            if (!TargetMetadata) continue; // Should not happen if TemplateMetadata is valid

            // Find a matching initialization parameter based on type
            bool bFoundMatchingParam = false;
            for (const FInstancedStruct& InitParam : InitParams)
            {
                // --- Type Matching Placeholder ---
                // Ideally, check type compatibility here:
                // if (TargetMetadata->CanInitializeFrom(InitParam))
                // For now, we assume InitializeFromParams handles mismatch or we try the first valid param.
                if (InitParam.IsValid()) // Simple check: Is the param data valid?
                {
                    // Initialize using the first matching parameter found
                    TargetMetadata->InitializeFromParams(InitParam);
                    bFoundMatchingParam = true;
                    // Assuming one init param per metadata type is intended in the InitParams array
                    break; 
                }
            }

            // Handle case where no suitable init param was found for this metadata type
            if (!bFoundMatchingParam)
            {
                // If the instance was newly added by FindOrAddLocalMetadataInstance, it has defaults.
                // If it existed, it retains its old state.
                UE_LOG(LogTemp, Verbose, TEXT("FSyStateCategories::ApplyInitData: No suitable initialization parameter found in provided InitParams for metadata type '%s' for tag '%s'. Instance will use defaults or retain existing state."),
                       *TargetMetadata->GetClass()->GetName(), *StateTag.ToString());
            }
        }
    }
}

void FSyStateCategories::ApplyStateModifications(const TMap<FGameplayTag, TArray<FInstancedStruct>>& StateModifications)
{
    // Iterate through all modification intentions (Tag + ModParams)
    for (const auto& StatePair : StateModifications)
    {
        const FGameplayTag& StateTag = StatePair.Key;
        const TArray<FInstancedStruct>& ModificationParams = StatePair.Value;

        if (ModificationParams.IsEmpty()) continue;

        // Get or create local storage for this tag
        FSyStateMetadatas& LocalMetadataArray = StateData.FindOrAdd(StateTag);
        
        // Get metadata instances defined for this tag by the system (UDS)
        TArray<UO_TagMetadata*> TagDefinedInstances = UDS_TagMetadata::GetTagMetadata(StateTag);

        // Iterate through each metadata type defined for the tag
        for (UO_TagMetadata* TagDefinedInstance : TagDefinedInstances)
        {
            USyStateMetadataBase* TemplateMetadata = Cast<USyStateMetadataBase>(TagDefinedInstance);
            if (!TemplateMetadata) continue;

            // Find or add the corresponding local instance
            USyStateMetadataBase* TargetInstance = FindOrAddLocalMetadataInstance(LocalMetadataArray, TemplateMetadata, StateTag);
            if (!TargetInstance) continue;

            // Apply modifications - let the instance decide if a param applies
            for (const FInstancedStruct& ModParam : ModificationParams)
            {
                // ApplyModification should internally check if ModParam type is relevant
                TargetInstance->ApplyModification(ModParam);
            }
        }
    }
}
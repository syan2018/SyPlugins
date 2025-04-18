// Copyright Epic Games, Inc. All Rights Reserved.

#include "StateContainerTypes.h"
#include "StateParameterTypes.h"
#include "StateMetadataTypes.h"
#include "DS_TagMetadata.h"
#include "Logging/LogMacros.h"
#include "UObject/Package.h"

DEFINE_LOG_CATEGORY_STATIC(LogSyStateCategories, Log, All);

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
    Empty(); // Clear existing data first

    // Iterate through InitData map {Tag -> Array<FInstancedStruct>}
    for (const auto& Pair : InitData.GetParametersAsMap())
    {
        const FGameplayTag& StateTag = Pair.Key;
        const TArray<FInstancedStruct>& InitParamsForTag = Pair.Value;

        // Get the TEMPLATE metadata objects defined for this tag (defines expected TYPES)
        TArray<UO_TagMetadata*> TemplateInstances = UDS_TagMetadata::GetTagMetadata(StateTag); // Assuming this returns correct templates

        FSyStateMetadatas& CurrentMetadatas = StateData.FindOrAdd(StateTag);
        CurrentMetadatas.MetadataArray.Empty(); // Start fresh for this tag

        // Iterate through the TEMPLATE types expected for this tag
        for (UO_TagMetadata* TemplateInstance : TemplateInstances)
        {
            USyStateMetadataBase* TemplateMetadata = Cast<USyStateMetadataBase>(TemplateInstance);
            if (!TemplateMetadata) continue;

            UClass* ExpectedMetadataClass = TemplateMetadata->GetClass();
            UScriptStruct* ExpectedValueType = TemplateMetadata->GetValueDataType();

            // Create a NEW instance of the correct metadata CLASS for this tag
            USyStateMetadataBase* NewMetadataInstance = NewObject<USyStateMetadataBase>(GetTransientPackage(), ExpectedMetadataClass); // Need Outer?
            if (!NewMetadataInstance)
            {
                UE_LOG(LogSyStateCategories, Error, TEXT("ApplyInitData: Failed to create metadata object of class %s for tag %s."), *ExpectedMetadataClass->GetName(), *StateTag.ToString());
                continue;
            }

            // Find the corresponding initialization parameter from the input data
            const FInstancedStruct* FoundInitParam = InitParamsForTag.FindByPredicate(
                [&ExpectedValueType](const FInstancedStruct& Param) {
                    return Param.IsValid() && Param.GetScriptStruct() == ExpectedValueType;
                });

            // Initialize the new instance with the found parameter (if any), otherwise it uses defaults
            if (FoundInitParam)
            {
                NewMetadataInstance->SetValueStruct(*FoundInitParam);
            }
            else
            {
                UE_LOG(LogSyStateCategories, Verbose, TEXT("ApplyInitData: No init param found for type %s (Metadata Class: %s) for tag %s. Using default value."),
                       *GetNameSafe(ExpectedValueType), *ExpectedMetadataClass->GetName(), *StateTag.ToString());
                // NewMetadataInstance already has default values from its constructor
            }

            // Add the newly created and initialized instance to the state
            CurrentMetadatas.MetadataArray.Add(NewMetadataInstance);
        }
    }
}

void FSyStateCategories::UpdateFromParameterMap(const TMap<FGameplayTag, TArray<FInstancedStruct>>& AggregatedParamsMap)
{
    // 1. Identify and remove tags no longer present in the aggregated map
    TArray<FGameplayTag> TagsToRemove;
    for (const auto& CurrentPair : StateData)
    {
        if (!AggregatedParamsMap.Contains(CurrentPair.Key))
        {
            TagsToRemove.Add(CurrentPair.Key);
        }
    }
    for (const FGameplayTag& Tag : TagsToRemove)
    {
        StateData.Remove(Tag);
        UE_LOG(LogSyStateCategories, Verbose, TEXT("UpdateFromParameterMap: Removing state tag %s."), *Tag.ToString());
    }

    // 2. Iterate through the aggregated map to update existing tags or add new ones
    for (const auto& AggregatedPair : AggregatedParamsMap)
    {
        const FGameplayTag& StateTag = AggregatedPair.Key;
        const TArray<FInstancedStruct>& NewParamsForTag = AggregatedPair.Value;

        // Get the TEMPLATE metadata objects defined for this tag (defines expected TYPES)
        TArray<UO_TagMetadata*> TemplateInstances = UDS_TagMetadata::GetTagMetadata(StateTag); // Assuming this returns correct templates

        FSyStateMetadatas& CurrentMetadatas = StateData.FindOrAdd(StateTag);
        TArray<TObjectPtr<UO_TagMetadata>> OldMetadataObjects = CurrentMetadatas.MetadataArray; // Keep track of old objects
        TArray<TObjectPtr<UO_TagMetadata>> NewMetadataArray; // Build the new list for this tag
        TSet<int32> UsedOldIndices; // Track which old objects were updated/reused

        // Iterate through the TEMPLATE types expected for this tag
        for (UO_TagMetadata* TemplateInstance : TemplateInstances)
        {
            USyStateMetadataBase* TemplateMetadata = Cast<USyStateMetadataBase>(TemplateInstance);
            if (!TemplateMetadata) continue;

            UClass* ExpectedMetadataClass = TemplateMetadata->GetClass();
            UScriptStruct* ExpectedValueType = TemplateMetadata->GetValueDataType();

            // Find the corresponding aggregated parameter from the input map for this specific type
            const FInstancedStruct* FoundAggregatedParam = NewParamsForTag.FindByPredicate(
                [&ExpectedValueType](const FInstancedStruct& Param) {
                    return Param.IsValid() && Param.GetScriptStruct() == ExpectedValueType;
                });

            if (FoundAggregatedParam) // A value for this type exists in the aggregated state
            {
                // Try to find an existing local instance of the correct class
                UO_TagMetadata* ExistingInstanceToUpdate = nullptr;
                int32 FoundOldIndex = INDEX_NONE;
                for(int32 i = 0; i < OldMetadataObjects.Num(); ++i)
                {
                    // Check if the index hasn't been used and the object is valid and of the correct CLASS
                    if (!UsedOldIndices.Contains(i) && OldMetadataObjects[i] && OldMetadataObjects[i]->IsA(ExpectedMetadataClass))
                    {
                        ExistingInstanceToUpdate = OldMetadataObjects[i];
                        FoundOldIndex = i;
                        break;
                    }
                }

                if (ExistingInstanceToUpdate) // Reuse existing instance
                {
                    Cast<USyStateMetadataBase>(ExistingInstanceToUpdate)->SetValueStruct(*FoundAggregatedParam);
                    NewMetadataArray.Add(ExistingInstanceToUpdate); // Add the updated instance to the new list
                    UsedOldIndices.Add(FoundOldIndex); // Mark index as used
                    UE_LOG(LogSyStateCategories, Verbose, TEXT("UpdateFromParameterMap: Updated existing metadata %s for tag %s."), *ExpectedMetadataClass->GetName(), *StateTag.ToString());
                }
                else // Create new instance
                {
                    USyStateMetadataBase* NewMetadataInstance = NewObject<USyStateMetadataBase>(GetTransientPackage(), ExpectedMetadataClass); // Need Outer?
                    if (NewMetadataInstance)
                    {
                        NewMetadataInstance->SetValueStruct(*FoundAggregatedParam);
                        NewMetadataArray.Add(NewMetadataInstance); // Add the new instance to the new list
                        UE_LOG(LogSyStateCategories, Verbose, TEXT("UpdateFromParameterMap: Created new metadata %s for tag %s."), *ExpectedMetadataClass->GetName(), *StateTag.ToString());
                    }
                    else { UE_LOG(LogSyStateCategories, Error, TEXT("UpdateFromParameterMap: Failed to create metadata %s for tag %s."), *ExpectedMetadataClass->GetName(), *StateTag.ToString()); }
                }
            }
            else // No aggregated value for this type exists (e.g., due to unloading).
            {
                 // Simply don't add any instance of this ExpectedMetadataClass to NewMetadataArray.
                 // Any existing instance in OldMetadataObjects of this class that isn't reused will be GC'd.
                 UE_LOG(LogSyStateCategories, Verbose, TEXT("UpdateFromParameterMap: No aggregated value for metadata type %s for tag %s. Instance (if any) removed/reset."), *ExpectedMetadataClass->GetName(), *StateTag.ToString());
            }
        }
        // Replace the old metadata array for this tag with the newly constructed one
        CurrentMetadatas.MetadataArray = NewMetadataArray;
    }
}

void FSyStateCategories::MergeWith(const FSyStateCategories& Other)
{
    // Keeping the simple MergeWith implementation as requested previously.
    // Be mindful of potential inconsistencies with UpdateFromParameterMap's optimization.
    for (const auto& Pair : Other.StateData)
    {
        const FGameplayTag& StateTag = Pair.Key;
        const FSyStateMetadatas& OtherMetadatas = Pair.Value;
        StateData.FindOrAdd(StateTag) = OtherMetadatas;
    }
}




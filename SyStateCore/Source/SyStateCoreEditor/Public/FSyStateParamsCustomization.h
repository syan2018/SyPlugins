// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyHandle.h"
#include "GameplayTagContainer.h" // For FGameplayTag
#include "Containers/Ticker.h" // Needed for FTSTicker
// Forward declarations to avoid including heavy headers if possible
struct FInstancedStruct;
struct FSyStateParams;
class USyStateMetadataBase;
class STextBlock;
class IPropertyUtilities;
class IDetailChildrenBuilder;
class FDetailWidgetRow;
class IPropertyHandleArray;


/**
 * Customizes the details panel view for the FSyStateParams struct.
 * Handles updating the 'Params' array based on the selected 'Tag'.
 * Also provides feedback if a duplicate tag is detected within the parent array.
 */
class SYSTATECOREEDITOR_API FSyStateParamsCustomization : public IPropertyTypeCustomization
{
public:
    static TSharedRef<IPropertyTypeCustomization> MakeInstance();
    virtual ~FSyStateParamsCustomization(); // Add destructor for cleanup

    /** IPropertyTypeCustomization interface */
    virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
    virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
    /** Callback when the 'Tag' property value changes. Schedules a deferred update. */
    void OnTagChanged();
    /** Schedules a deferred update of the Params array for the given tag. */
    void RequestDeferredUpdate(const FGameplayTag& NewTag);
    /** Function called by the ticker to perform the deferred update. */
    bool DeferredUpdateParams(float DeltaTime);

    /** Updates the 'Params' TArray based on the provided GameplayTag. (Internal logic) */
    void UpdateParamsForTag_Internal(const FGameplayTag& Tag);

    /** Gets the property handle for the 'Tag' member of the struct. */
    TSharedPtr<IPropertyHandle> GetTagPropertyHandle() const;
    /** Gets the property handle for the 'Params' member of the struct. */
    TSharedPtr<IPropertyHandle> GetParamsPropertyHandle() const;

    /** Attempts to find the parent IPropertyHandleArray for duplication checks. */
    TSharedPtr<IPropertyHandleArray> GetParentArrayHandle() const;

    /** Checks if the given Tag is already present in other elements of the parent array. */
    bool IsTagDuplicate(const FGameplayTag& TagToCheck, int32& OutDuplicateIndex) const;

    /** Shows the warning message about a duplicate tag. */
    void ShowDuplicateTagWarning(const FGameplayTag& DuplicateTag, int32 DuplicateIndex);
    /** Hides the duplicate tag warning message. */
    void HideDuplicateTagWarning();

    // --- Helper functions moved from the previous customization --- 
    /** Serializes an FInstancedStruct into a property handle's data. */
    void SerializeParameterStruct(const FInstancedStruct& Instance, const TSharedRef<IPropertyHandle>& Handle);
    /** Creates an FInstancedStruct based on metadata. */
    FInstancedStruct CreateParameterStructFromMetadata(const USyStateMetadataBase* StateMetadata);

    /** Handle to the struct being customized. */
    TSharedPtr<IPropertyHandle> StructHandle;
    /** Cached handle to the 'Tag' property. */
    TSharedPtr<IPropertyHandle> TagHandle;
    /** Cached handle to the 'Params' property. */
    TSharedPtr<IPropertyHandle> ParamsHandle;
    /** Property utilities provided by the customization system. */
    TSharedPtr<IPropertyUtilities> PropertyUtilities;
    /** Text block used to display the duplicate tag warning. */
    TSharedPtr<STextBlock> WarningTextBlock;
    /** Holds the currently detected duplicate tag for warning display. */
    FGameplayTag CurrentDuplicateTag;

    /** Ticker handle for deferred update */
    FTSTicker::FDelegateHandle DeferredUpdateTickerHandle;
    /** Tag value captured for deferred update */
    FGameplayTag TagForDeferredUpdate;
    /** The tag value for which the Params array was last successfully updated or cleared. */
    FGameplayTag LastProcessedTag = FGameplayTag::EmptyTag; // Initialize properly
}; 
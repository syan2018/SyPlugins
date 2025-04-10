// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyHandle.h"
#include "Widgets/Text/STextBlock.h"
#include "DS_TagMetadata.h" // For UDS_TagMetadata

class FSyStateParameterSetCustomization : public IPropertyTypeCustomization
{
public:
    static TSharedRef<IPropertyTypeCustomization> MakeInstance();

    /** IPropertyTypeCustomization interface */
    virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
    virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
    // Handles customization for a single Tag -> Params pair in the TMap
    void CustomizeMapElement(TSharedRef<IPropertyHandle> MapPropertyHandle, TSharedRef<IPropertyHandle> KeyHandle, TSharedRef<IPropertyHandle> ValueHandle);

    // Callback when the GameplayTag (key) value changes
    void OnTagChanged(TSharedRef<IPropertyHandle> ValueHandle);

    // Updates the FSyStateParams struct based on the selected tag
    void UpdateParameterStructForTag(const FGameplayTag& Tag, TSharedRef<IPropertyHandle> ParamsPropertyHandle);

    TSharedPtr<IPropertyHandle> StructHandle;
    uint32 PreviousNumElements = 0;
    uint32 CurrentNumElements = 0;
}; 
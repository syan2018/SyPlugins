// Copyright Epic Games, Inc. All Rights Reserved.

#include "FSyStateParamsCustomization.h"
#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "PropertyHandle.h"
#include "IPropertyUtilities.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Styling/AppStyle.h"
#include "ScopedTransaction.h" // For Undo/Redo

// Include necessary headers for types used
#include "DetailWidgetRow.h"
#include "StateParameterTypes.h" // Defines FSyStateParams
#include "StateMetadataTypes.h"  // Defines USyStateMetadataBase
#include "DS_TagMetadata.h"      // Defines UDS_TagMetadata
#include "SyCore/Public/Foundation/Utilities/SyInstancedStruct.h" // Defines FSyInstancedStruct

#define LOCTEXT_NAMESPACE "FSyStateParamsCustomization"

TSharedRef<IPropertyTypeCustomization> FSyStateParamsCustomization::MakeInstance()
{
    return MakeShareable(new FSyStateParamsCustomization());
}

void FSyStateParamsCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    // Use default header rendering provided by the parent structure (likely TArray)
    // We will customize children instead.
    HeaderRow.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	];
    // Optionally hide the header if the array element title is sufficient
	// HeaderRow.Visibility(EVisibility::Collapsed);
}

void FSyStateParamsCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    StructHandle = StructPropertyHandle;
    PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();

    // Get handles for Tag and Params members
    TagHandle = GetTagPropertyHandle();
    ParamsHandle = GetParamsPropertyHandle();

    if (!TagHandle.IsValid() || !TagHandle->IsValidHandle())
    {
        UE_LOG(LogTemp, Error, TEXT("FSyStateParamsCustomization: Could not find 'Tag' property handle."));
        return;
    }
    if (!ParamsHandle.IsValid() || !ParamsHandle->IsValidHandle())
    { 
        UE_LOG(LogTemp, Error, TEXT("FSyStateParamsCustomization: Could not find 'Params' property handle."));
        return;
    }

    // Add the 'Tag' property row
    IDetailPropertyRow& TagRow = ChildBuilder.AddProperty(TagHandle.ToSharedRef());
    // Bind delegates to the Tag property changes
    // Using PostChange might be more reliable for getting the final value
    TagHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FSyStateParamsCustomization::OnTagChanged));
    // TagHandle->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FSyStateParamsCustomization::OnTagChanged)); // Alternative if nested changes need watching
    // TagHandle->SetOnPropertyValueChangedWithData( TDelegate<void(const FPropertyChangedEvent&)>::CreateSP(this, &FSyStateParamsCustomization::OnTagChanged_Post) ); // More complex, provides change event data

    // Add a row for the duplicate tag warning message below the Tag property
    ChildBuilder.AddCustomRow(LOCTEXT("DuplicateTagWarningLabel", "DuplicateWarning"))
    .Visibility(EVisibility::Collapsed) // Initially hidden
    .NameContent()
    [
        SNew(SImage)
        .Image(FAppStyle::Get().GetBrush("Icons.WarningWithColor"))
        .ToolTipText(LOCTEXT("DuplicateTagWarningTooltip", "This tag is already used by another element in the array."))
    ]
    .ValueContent()
    [
        SAssignNew(WarningTextBlock, STextBlock)
        .Font(IDetailLayoutBuilder::GetDetailFontBold())
        .ColorAndOpacity(FAppStyle::Get().GetSlateColor("Colors.Warning"))
        // .Text(LOCTEXT("DuplicateTagWarningText", "Duplicate Tag!")) // Text set dynamically
    ];

    // Add the 'Params' property row (TArray)
    // Use default array handling for now
    IDetailPropertyRow& ParamsRow = ChildBuilder.AddProperty(ParamsHandle.ToSharedRef());

    // Initial check for duplicates and update Params based on initial Tag value
    OnTagChanged_Post(); // Call post-change logic initially
}

void FSyStateParamsCustomization::OnTagChanged()
{
    // This might be called before the value is fully committed. 
    // We set a flag and defer the actual logic to OnTagChanged_Post, 
    // which we can potentially trigger via a timer or another mechanism if PostChange delegate isn't reliable.
    // For now, let's assume a direct call to Post logic might work for struct members.
    OnTagChanged_Post();

    // Request a refresh of the details panel to potentially show/hide the warning immediately
    if(PropertyUtilities.IsValid())
    {
        PropertyUtilities->RequestRefresh();
    }
}

void FSyStateParamsCustomization::OnTagChanged_Post()
{
    if (!TagHandle.IsValid())
    {
        return;
    }

    FGameplayTag CurrentTag;
    void* TagData = nullptr;
    FPropertyAccess::Result Result = TagHandle->GetValueData(TagData);

    if (Result == FPropertyAccess::Success && TagData)
    {        
        CurrentTag = *static_cast<FGameplayTag*>(TagData);
        
        if(CurrentTag.IsValid())
        {
            int32 DuplicateIndex = INDEX_NONE;
            if (IsTagDuplicate(CurrentTag, DuplicateIndex))
            {
                ShowDuplicateTagWarning(CurrentTag, DuplicateIndex);
                // Policy: Still update Params even if duplicate for now
                UpdateParamsForTag(CurrentTag);
            }
            else
            {            
                HideDuplicateTagWarning();
                UpdateParamsForTag(CurrentTag);
            }
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("FSyStateParamsCustomization::OnTagChanged_Post: Tag value read is None or invalid."));
            HideDuplicateTagWarning();
            UpdateParamsForTag(CurrentTag);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("FSyStateParamsCustomization::OnTagChanged_Post: Failed to get Tag value data (Result: %d)."), static_cast<int32>(Result));
        HideDuplicateTagWarning();
        // Optionally clear Params if Tag is invalid or unreadable?
        // UpdateParamsForTag(FGameplayTag()); // Pass invalid tag to clear
    }
}

TSharedPtr<IPropertyHandle> FSyStateParamsCustomization::GetTagPropertyHandle() const
{
    return StructHandle.IsValid() ? StructHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSyStateParams, Tag)) : nullptr;
}

TSharedPtr<IPropertyHandle> FSyStateParamsCustomization::GetParamsPropertyHandle() const
{
    return StructHandle.IsValid() ? StructHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSyStateParams, Params)) : nullptr;
}

TSharedPtr<IPropertyHandleArray> FSyStateParamsCustomization::GetParentArrayHandle() const
{
    if (StructHandle.IsValid())
    {
        TSharedPtr<IPropertyHandle> ParentHandle = StructHandle->GetParentHandle();
        if (ParentHandle.IsValid() && ParentHandle->AsArray().IsValid())
        {
            return ParentHandle->AsArray();
        }
    }
    return nullptr;
}

bool FSyStateParamsCustomization::IsTagDuplicate(const FGameplayTag& TagToCheck, int32& OutDuplicateIndex) const
{
    OutDuplicateIndex = INDEX_NONE;
    if (!TagToCheck.IsValid()) return false; // Invalid tags cannot be duplicates in this context

    TSharedPtr<IPropertyHandleArray> ParentArrayHandle = GetParentArrayHandle();
    if (!ParentArrayHandle.IsValid()) return false;

    uint32 NumParentElements = 0;
    ParentArrayHandle->GetNumElements(NumParentElements);

    int32 MyIndex = StructHandle->GetIndexInArray();

    for (uint32 i = 0; i < NumParentElements; ++i)
    {
        if (static_cast<int32>(i) == MyIndex) continue; // Don't compare with self

        TSharedPtr<IPropertyHandle> SiblingStructHandle = ParentArrayHandle->GetElement(i);
        if (SiblingStructHandle.IsValid())
        {
            TSharedPtr<IPropertyHandle> SiblingTagHandle = SiblingStructHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSyStateParams, Tag));
            if (SiblingTagHandle.IsValid() && SiblingTagHandle->IsValidHandle())
            {
                FGameplayTag SiblingTag;
                void* SiblingTagData = nullptr;
                if (SiblingTagHandle->GetValueData(SiblingTagData) == FPropertyAccess::Success && SiblingTagData)
                {
                    SiblingTag = *static_cast<FGameplayTag*>(SiblingTagData);
                    if (SiblingTag == TagToCheck)
                    {
                        OutDuplicateIndex = i;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

void FSyStateParamsCustomization::ShowDuplicateTagWarning(const FGameplayTag& DuplicateTag, int32 DuplicateIndex)
{
    if (WarningTextBlock.IsValid())
    {
        FString WarningMsg = FString::Printf(TEXT("Duplicate Tag! Already used by element %d."), DuplicateIndex);
        WarningTextBlock->SetText(FText::FromString(WarningMsg));
        WarningTextBlock->SetVisibility(EVisibility::Visible);
        CurrentDuplicateTag = DuplicateTag; // Store for potential later checks
    }
}

void FSyStateParamsCustomization::HideDuplicateTagWarning()
{
    if (WarningTextBlock.IsValid())
    {
        WarningTextBlock->SetVisibility(EVisibility::Collapsed);
        CurrentDuplicateTag = FGameplayTag(); // Clear stored duplicate
    }
}


// --- Helper Function Implementations (from previous customization, adjusted) ---

void FSyStateParamsCustomization::SerializeParameterStruct(const FInstancedStruct& Instance, const TSharedRef<IPropertyHandle>& Handle)
{
    void* ElementData = nullptr;
    if (Handle->GetValueData(ElementData) == FPropertyAccess::Success && ElementData)
    {        
        *(static_cast<FInstancedStruct*>(ElementData)) = Instance;
        // Notify change on the specific element handle
        Handle->NotifyFinishedChangingProperties();
    }
    else
    {
         UE_LOG(LogTemp, Warning, TEXT("FSyStateParamsCustomization::SerializeParameterStruct: Failed to get value data for handle."));
    }
}

FInstancedStruct FSyStateParamsCustomization::CreateParameterStructFromMetadata(const USyStateMetadataBase* StateMetadata)
{
    FInstancedStruct NewInstance;
    if (StateMetadata && StateMetadata->GetValueDataType())
    {
        NewInstance.InitializeAs(StateMetadata->GetValueDataType());
        // TODO: Optionally initialize default values from metadata if needed
    }
    return NewInstance;
}

void FSyStateParamsCustomization::UpdateParamsForTag(const FGameplayTag& Tag)
{
    if (!ParamsHandle.IsValid() || !ParamsHandle->AsArray().IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("FSyStateParamsCustomization::UpdateParamsForTag: Invalid Params array handle."));
        return;
    }

    TSharedRef<IPropertyHandleArray> ArrayHandleRef = ParamsHandle->AsArray().ToSharedRef();

    // Use ScopedTransaction for undo/redo
    const FScopedTransaction Transaction(LOCTEXT("UpdateParamsForTagTransaction", "Update State Params Array"));
    // Notify the array property is about to change
    ParamsHandle->NotifyPreChange(); 
    
    ArrayHandleRef->EmptyArray();

    if (Tag.IsValid())
    {
        TArray<UO_TagMetadata*> AllMetadata = UDS_TagMetadata::GetTagMetadata(Tag);
        TArray<USyStateMetadataBase*> StateMetadataList;

        for (UO_TagMetadata* Metadata : AllMetadata)
        {
            if (USyStateMetadataBase* StateMetadata = Cast<USyStateMetadataBase>(Metadata))
            {
                StateMetadataList.Add(StateMetadata);
            }
        }
        
        if (StateMetadataList.Num() == 0)
        {
             UE_LOG(LogTemp, Log, TEXT("FSyStateParamsCustomization: No USyStateMetadataBase found for tag %s. Params array will be empty."), *Tag.ToString());
        }

        for (USyStateMetadataBase* StateMetadata : StateMetadataList)
        {
            if (StateMetadata && StateMetadata->GetValueDataType())
            {
                ArrayHandleRef->AddItem();
                uint32 NumElements;
                ArrayHandleRef->GetNumElements(NumElements);
                
                if (NumElements > 0)
                {
                    TSharedPtr<IPropertyHandle> NewElementHandle = ArrayHandleRef->GetElement(NumElements - 1);
                    if (NewElementHandle.IsValid() && NewElementHandle->IsValidHandle())
                    {
                        FInstancedStruct NewInstance = CreateParameterStructFromMetadata(StateMetadata);
                        // Ensure the created instance is valid before serializing
                        if (NewInstance.IsValid())
                        {
                            SerializeParameterStruct(NewInstance, NewElementHandle.ToSharedRef());
                        }
                        else
                        {
                            UE_LOG(LogTemp, Warning, TEXT("FSyStateParamsCustomization: Created invalid FInstancedStruct from metadata for tag %s."), *Tag.ToString());
                            // Remove the potentially invalid item we just added
                            ArrayHandleRef->DeleteItem(NumElements - 1); 
                        }
                    }
                    else
                    {
                         UE_LOG(LogTemp, Warning, TEXT("FSyStateParamsCustomization: Failed to get handle for new array element %d for tag %s."), NumElements - 1, *Tag.ToString());
                    }
                }
            }
            else
            {
                 UE_LOG(LogTemp, Warning, TEXT("FSyStateParamsCustomization: Skipping invalid StateMetadata or ValueDataType for tag %s."), *Tag.ToString());
            }
        }
    }
    else
    {
         UE_LOG(LogTemp, Log, TEXT("FSyStateParamsCustomization: Tag is invalid, clearing Params array."));
    }
    
    // Notify that the array property change is complete
    ParamsHandle->NotifyPostChange(EPropertyChangeType::ArrayAdd); // Use appropriate change type
    ParamsHandle->NotifyFinishedChangingProperties();
    // Also notify the parent struct handle that its child changed
    StructHandle->NotifyFinishedChangingProperties(); 
}

#undef LOCTEXT_NAMESPACE 
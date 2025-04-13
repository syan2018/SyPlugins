// Copyright Epic Games, Inc. All Rights Reserved.

#include "FSyStateParamsCustomization.h"
#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "PropertyHandle.h"
#include "StructUtils/InstancedStruct.h"
#include "IPropertyUtilities.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Styling/AppStyle.h"
#include "ScopedTransaction.h" // For Undo/Redo
#include "Containers/Ticker.h" // Make sure Ticker is included

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

FSyStateParamsCustomization::~FSyStateParamsCustomization()
{
    if (DeferredUpdateTickerHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(DeferredUpdateTickerHandle);
        DeferredUpdateTickerHandle.Reset();
    }
}

void FSyStateParamsCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    HeaderRow.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	];
}

void FSyStateParamsCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    StructHandle = StructPropertyHandle;
    PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();

    TagHandle = GetTagPropertyHandle();
    ParamsHandle = GetParamsPropertyHandle();

    if (!TagHandle.IsValid() || !TagHandle->IsValidHandle() || !ParamsHandle.IsValid() || !ParamsHandle->IsValidHandle())
    {
        UE_LOG(LogTemp, Error, TEXT("FSyStateParamsCustomization: Could not find required property handles (Tag or Params)."));
        return;
    }

    // Add the 'Tag' property row using default widgets
    ChildBuilder.AddProperty(TagHandle.ToSharedRef())
        .ShouldAutoExpand(true);
        
    // 不再需要绑定 OnTagChanged，因为 PostSerialize 会处理 Tag 变更

    // Add warning row
    ChildBuilder.AddCustomRow(LOCTEXT("DuplicateTagWarningLabel", "DuplicateWarning"))
    .Visibility(EVisibility::Collapsed)
    .WholeRowContent()
    [
        SNew(SHorizontalBox)
        +SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(2.0f, 0.0f)
        .VAlign(VAlign_Center)
        [
            SNew(SImage)
            .Image(FAppStyle::Get().GetBrush("Icons.WarningWithColor"))
            .ToolTipText(LOCTEXT("DuplicateTagWarningTooltip", "This tag is already used by another element in the array."))
        ]
        +SHorizontalBox::Slot()
        .FillWidth(1.0f)
        .VAlign(VAlign_Center)
        [
            SAssignNew(WarningTextBlock, STextBlock)
            .Font(IDetailLayoutBuilder::GetDetailFontBold())
            .ColorAndOpacity(FAppStyle::Get().GetSlateColor("Colors.Warning"))
        ]
    ];

    // 添加 Params 数组标题
    ChildBuilder.AddCustomRow(LOCTEXT("ParamsArrayLabel", "Parameters"))
    .NameContent()
    [
        SNew(STextBlock)
        .Text(LOCTEXT("ParamsArrayLabel", "Parameters"))
        .Font(IDetailLayoutBuilder::GetDetailFont())
    ];

    // 添加参数数组
    IDetailPropertyRow& ParamsRow = ChildBuilder.AddProperty(ParamsHandle.ToSharedRef());
    ParamsRow
        .ShowPropertyButtons(true)
        .ShouldAutoExpand(true);

    // 自定义每个参数的显示
    uint32 NumParams = 0;
    if (ParamsHandle->AsArray()->GetNumElements(NumParams) == FPropertyAccess::Success)
    {
        for (uint32 ParamIndex = 0; ParamIndex < NumParams; ++ParamIndex)
        {
            TSharedRef<IPropertyHandle> ParamHandle = ParamsHandle->AsArray()->GetElement(ParamIndex);
            
            // 获取参数类型信息
            void* ParamData = nullptr;
            if (ParamHandle->GetValueData(ParamData) == FPropertyAccess::Success && ParamData)
            {
                FSyInstancedStruct* InstancedStruct = static_cast<FSyInstancedStruct*>(ParamData);
                if (InstancedStruct && InstancedStruct->GetScriptStruct())
                {
                    FString ParamTypeName = InstancedStruct->GetScriptStruct()->GetName();
                    
                    // 添加自定义行显示参数
                    IDetailPropertyRow& ElementRow = ChildBuilder.AddProperty(ParamHandle);
                    ElementRow
                        .DisplayName(FText::FromString(FString::Printf(TEXT("Parameter %d (%s)"), ParamIndex, *ParamTypeName)))
                        .ShowPropertyButtons(true)
                        .EditCondition(true, nullptr)
                        .ShouldAutoExpand(true);
                }
            }
        }
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
    if (!StructHandle.IsValid())
    {        
        return nullptr; // Error logged elsewhere or implicitly handled
    }
    TSharedPtr<IPropertyHandle> ParentHandle = StructHandle->GetParentHandle();
    if (!ParentHandle.IsValid())
    {        
        return nullptr; // Error logged elsewhere or implicitly handled
    }
    if (!ParentHandle->AsArray().IsValid())
    {        
        TSharedPtr<IPropertyHandle> GrandParentHandle = ParentHandle->GetParentHandle();
        if(GrandParentHandle.IsValid() && GrandParentHandle->AsArray().IsValid())
        {
            return GrandParentHandle->AsArray();
        }
    }
    
    return ParentHandle->AsArray();
}

bool FSyStateParamsCustomization::IsTagDuplicate(const FGameplayTag& TagToCheck, int32& OutDuplicateIndex) const
{
    OutDuplicateIndex = INDEX_NONE;
    if (!TagToCheck.IsValid()) return false; 

    TSharedPtr<IPropertyHandleArray> ParentArrayHandle = GetParentArrayHandle();
    if (!ParentArrayHandle.IsValid()) 
    {        
        // Error already logged in GetParentArrayHandle
        return false;
    }

    uint32 NumParentElements = 0;
    if(ParentArrayHandle->GetNumElements(NumParentElements) != FPropertyAccess::Success)
    {
        UE_LOG(LogTemp, Warning, TEXT("FSyStateParamsCustomization::IsTagDuplicate: Failed to get number of elements from parent array."));
        return false;
    }

    int32 MyIndex = StructHandle->GetIndexInArray(); 
    if (MyIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Error, TEXT("FSyStateParamsCustomization::IsTagDuplicate: Could not get current struct index in parent array."));
        return false; 
    }

    for (uint32 i = 0; i < NumParentElements; ++i)
    {
        if (static_cast<int32>(i) == MyIndex) continue; 

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
                    if (SiblingTag.IsValid() && SiblingTag == TagToCheck) 
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
        CurrentDuplicateTag = DuplicateTag; 
    }
}

void FSyStateParamsCustomization::HideDuplicateTagWarning()
{
    if (WarningTextBlock.IsValid())
    {
        WarningTextBlock->SetVisibility(EVisibility::Collapsed);
        CurrentDuplicateTag = FGameplayTag(); 
    }
}

void FSyStateParamsCustomization::SerializeParameterStruct(const FInstancedStruct& Instance, const TSharedRef<IPropertyHandle>& Handle)
{
    void* ElementData = nullptr;
    if (Handle->GetValueData(ElementData) == FPropertyAccess::Success && ElementData)
    {        
        *(static_cast<FInstancedStruct*>(ElementData)) = Instance;
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
    }
    return NewInstance;
}

void FSyStateParamsCustomization::UpdateParamsForTag_Internal(const FGameplayTag& Tag)
{
    if (!ParamsHandle.IsValid() || !ParamsHandle->AsArray().IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("FSyStateParamsCustomization::UpdateParamsForTag_Internal: Invalid Params array handle."));
        return;
    }
    
    if(StructHandle.IsValid() && StructHandle->IsEditConst())
    {
        UE_LOG(LogTemp, Verbose, TEXT("FSyStateParamsCustomization::UpdateParamsForTag_Internal: Parent struct is read-only, cannot modify Params."));
        return;
    }

    TSharedRef<IPropertyHandleArray> ArrayHandleRef = ParamsHandle->AsArray().ToSharedRef();
    
    const FScopedTransaction Transaction(LOCTEXT("UpdateParamsForTagTransaction", "Update State Params Array"));
    StructHandle->NotifyPreChange();
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
                        if (NewInstance.IsValid())
                        {
                            SerializeParameterStruct(NewInstance, NewElementHandle.ToSharedRef());
                        }
                        else
                        {                            
                            UE_LOG(LogTemp, Warning, TEXT("FSyStateParamsCustomization: Created invalid FInstancedStruct from metadata %s for tag %s."), *GetNameSafe(StateMetadata), *Tag.ToString());
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
    
    ParamsHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
    ParamsHandle->NotifyFinishedChangingProperties();
    StructHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
    StructHandle->NotifyFinishedChangingProperties();
}

#undef LOCTEXT_NAMESPACE 
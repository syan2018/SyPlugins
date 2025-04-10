// Copyright Epic Games, Inc. All Rights Reserved.

#include "FSyStateParameterSetCustomization.h"
#include "StateParameterTypes.h" // For FSyStateParameterSet, FSyStateParams
#include "IDetailPropertyRow.h"
#include "PropertyCustomizationHelpers.h"
#include "GameplayTagContainer.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "StateMetadataTypes.h"

#define LOCTEXT_NAMESPACE "FSyStateParameterSetCustomization"

TSharedRef<IPropertyTypeCustomization> FSyStateParameterSetCustomization::MakeInstance()
{
    return MakeShareable(new FSyStateParameterSetCustomization());
}

void FSyStateParameterSetCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    // We don't need a fancy header, default struct header is fine.
    HeaderRow.NameContent()
    [
        StructPropertyHandle->CreatePropertyNameWidget()
    ];
}

void FSyStateParameterSetCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    StructHandle = StructPropertyHandle;

    uint32 NumChildren;
    StructPropertyHandle->GetNumChildren(NumChildren);

    for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
    {
        TSharedPtr<IPropertyHandle> ChildHandle = StructPropertyHandle->GetChildHandle(ChildIndex);
        if (ChildHandle.IsValid() && ChildHandle->GetProperty()->GetFName() == GET_MEMBER_NAME_CHECKED(FSyStateParameterSet, Parameters))
        {
            // Customize the TMap property
            TSharedRef<IPropertyHandleMap> MapHandle = ChildHandle->AsMap().ToSharedRef();
            
            // Add the default Add/Empty elements button
            ChildBuilder.AddProperty(ChildHandle.ToSharedRef()).ShowPropertyButtons(true);

            // Customize existing elements
            FSimpleDelegate OnRegenerateChildren = FSimpleDelegate::CreateLambda([MapHandle, this, &StructCustomizationUtils]() {
                uint32 NumElements;
                MapHandle->GetNumElements(NumElements);
                if (PreviousNumElements != NumElements)
                {
                    PreviousNumElements = NumElements;
                    // 当元素数量变化时，强制刷新整个详情面板
                    StructCustomizationUtils.GetPropertyUtilities()->ForceRefresh();
                }
            });
            MapHandle->SetOnNumElementsChanged(OnRegenerateChildren);

            uint32 NumElements;
            MapHandle->GetNumElements(NumElements);
            for (uint32 ElementIndex = 0; ElementIndex < NumElements; ++ElementIndex)
            {
                TSharedPtr<IPropertyHandle> ElementHandle = MapHandle->GetElement(ElementIndex);
                if (ElementHandle.IsValid())
                {
                    TSharedPtr<IPropertyHandle> KeyHandle = ElementHandle->GetKeyHandle();
                    TSharedPtr<IPropertyHandle> ValueHandle = ElementHandle->GetChildHandle(0); // The value is the FSyStateParams struct

                    if (KeyHandle.IsValid() && ValueHandle.IsValid())
                    {
                        CustomizeMapElement(ElementHandle.ToSharedRef(), KeyHandle.ToSharedRef(), ValueHandle.ToSharedRef());
                    }
                }
            }
            PreviousNumElements = NumElements;
            break; // Found and processed the map
        }
        else if(ChildHandle.IsValid())
        {
            // Add other properties as default
            ChildBuilder.AddProperty(ChildHandle.ToSharedRef());
        }
    }
}

void FSyStateParameterSetCustomization::CustomizeMapElement(TSharedRef<IPropertyHandle> ElementHandle, TSharedRef<IPropertyHandle> KeyHandle, TSharedRef<IPropertyHandle> ValueHandle)
{
    // Store ValueHandle in the delegate's lambda capture
    KeyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FSyStateParameterSetCustomization::OnTagChanged, ValueHandle));

    // Initial update in case the tag is already set
    FString CurrentTagString;
    if (KeyHandle->GetValueAsFormattedString(CurrentTagString) == FPropertyAccess::Success)
    {
        FGameplayTag CurrentTag = FGameplayTag::RequestGameplayTag(FName(*CurrentTagString));
        if (CurrentTag.IsValid())
        {
            UpdateParameterStructForTag(CurrentTag, ValueHandle);
        }
    }
}

void FSyStateParameterSetCustomization::OnTagChanged(TSharedRef<IPropertyHandle> ValueHandle)
{
    // Find the corresponding key handle for this value handle
    TSharedPtr<IPropertyHandle> ParentHandle = ValueHandle->GetParentHandle();
    if (!ParentHandle.IsValid()) return;

    TSharedPtr<IPropertyHandle> KeyHandle = ParentHandle->GetKeyHandle();
    if (!KeyHandle.IsValid()) return;

    // 获取标签值
    FString TagString;
    if (KeyHandle->GetValueAsFormattedString(TagString) == FPropertyAccess::Success)
    {
        FGameplayTag NewTag = FGameplayTag::RequestGameplayTag(FName(*TagString));
        if (NewTag.IsValid())
        {
            UpdateParameterStructForTag(NewTag, ValueHandle);
        }
    }
}

void FSyStateParameterSetCustomization::UpdateParameterStructForTag(const FGameplayTag& Tag, TSharedRef<IPropertyHandle> ParamsPropertyHandle)
{
    UScriptStruct* TargetStructType = nullptr;

    if (Tag.IsValid())
    {
        // 获取所有与标签关联的元数据
        TArray<UO_TagMetadata*> AllMetadata = UDS_TagMetadata::GetTagMetadata(Tag);
        
        // 遍历所有元数据，找到第一个 USyStateMetadataBase 类型的元数据
        for (UO_TagMetadata* Metadata : AllMetadata)
        {
            if (USyStateMetadataBase* StateMetadata = Cast<USyStateMetadataBase>(Metadata))
            {
                TargetStructType = StateMetadata->GetValueDataType();
                break;
            }
        }
    }
    
    // 3. Access the TArray<FSyInstancedStruct> property handle
    TSharedPtr<IPropertyHandle> ArrayHandle = ParamsPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSyStateParams, Params));
    if (!ArrayHandle.IsValid() || !ArrayHandle->IsValidHandle())
    {
         UE_LOG(LogTemp, Warning, TEXT("FSyStateParameterSetCustomization: Could not get 'Params' array handle."));
        return;
    }

    TSharedRef<IPropertyHandleArray> ArrayHandleRef = ArrayHandle->AsArray().ToSharedRef();

    // 4. Clear the array first
    ArrayHandleRef->EmptyArray();

    if (TargetStructType)
    {
        // 5. Add one new element
        ArrayHandleRef->AddItem();
        uint32 NumElements;
        ArrayHandleRef->GetNumElements(NumElements);
        if (NumElements > 0)
        {
             // 6. Get the handle for the new FSyInstancedStruct element
            TSharedPtr<IPropertyHandle> NewElementHandle = ArrayHandleRef->GetElement(NumElements - 1);
            if (NewElementHandle.IsValid() && NewElementHandle->IsValidHandle())
            {
                // 7. Set its underlying struct type
                UE_LOG(LogTemp, Log, TEXT("FSyStateParameterSetCustomization: Intending to set struct type for tag '%s' to '%s'"), *Tag.ToString(), *TargetStructType->GetName());

                // 创建新的实例化结构体
                FInstancedStruct NewInstance;
                NewInstance.InitializeAs(TargetStructType);

                // 获取原始数据指针
                void* ElementData = nullptr;
                if (NewElementHandle->GetValueData(ElementData) == FPropertyAccess::Success)
                {
                     if(ElementData)
                     {
                          *(static_cast<FInstancedStruct*>(ElementData)) = NewInstance;
                          // 使用 NotifyPostChange 替代 NotifyValueChanged
                          NewElementHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
                     }
                } 
                else {
                      UE_LOG(LogTemp, Warning, TEXT("FSyStateParameterSetCustomization: Failed to get value data for new array element."));
                }
            }
             else {
                 UE_LOG(LogTemp, Warning, TEXT("FSyStateParameterSetCustomization: Failed to get handle for new array element."));
             }
        }
         else {
             UE_LOG(LogTemp, Warning, TEXT("FSyStateParameterSetCustomization: Array size is not > 0 after AddItem."));
         }
    }
}

#undef LOCTEXT_NAMESPACE 
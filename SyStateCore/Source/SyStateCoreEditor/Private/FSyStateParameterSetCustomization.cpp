// Copyright Epic Games, Inc. All Rights Reserved.

#include "FSyStateParameterSetCustomization.h"

#include "DS_TagMetadata.h"
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
    PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();

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
    // 绑定标签变化委托
    KeyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([this, KeyHandle, ValueHandle]()
    {
        // 尝试获取属性数据的指针
        if (void* KeyData = nullptr; KeyHandle->GetValueData(KeyData) == FPropertyAccess::Success && KeyData)
        {
            // 将指针转换为 FGameplayTag 类型
            if (const FGameplayTag NewTag = *static_cast<FGameplayTag*>(KeyData); NewTag.IsValid())
            {
                CurrentTag = NewTag;
                // 在修改属性值后调用 UpdateParameterStructForTag
                UpdateParameterStructForTag(NewTag, ValueHandle);

                // 可选：如果需要，手动通知属性系统值已更改，以触发UI刷新或其他依赖项
                // KeyHandle->NotifyPropertyValueChanged(); 
            }
            else
            {
                 UE_LOG(LogTemp, Warning, TEXT("FSyStateParameterSetCustomization: Tag retrieved via GetValueData is invalid."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("FSyStateParameterSetCustomization: Failed to get value data for KeyHandle."));
        }
    }));

    // 初始更新 (这里仍然可以使用 GetValueAsFormattedString，因为这不是在值变化的回调中)
    FString InitialTagString;
    if (KeyHandle->GetValueAsFormattedString(InitialTagString) == FPropertyAccess::Success)
    {
        FGameplayTag InitialTag = FGameplayTag::RequestGameplayTag(FName(*InitialTagString), false);
        if (InitialTag.IsValid())
        {
            CurrentTag = InitialTag;
            UpdateParameterStructForTag(InitialTag, ValueHandle);
        }
        else
        {
            // 如果初始字符串无法解析为有效Tag，也尝试用 GetValueData 获取一次
            void* InitialKeyData = nullptr;
            if (KeyHandle->GetValueData(InitialKeyData) == FPropertyAccess::Success && InitialKeyData)
            {
                InitialTag = *static_cast<FGameplayTag*>(InitialKeyData);
                if (InitialTag.IsValid())
                {
                    CurrentTag = InitialTag;
                    UpdateParameterStructForTag(InitialTag, ValueHandle);
                }
            }
        }
    }
}

FInstancedStruct FSyStateParameterSetCustomization::CreateParameterStructFromMetadata(const USyStateMetadataBase* StateMetadata)
{
    FInstancedStruct NewInstance;
    if (StateMetadata && StateMetadata->GetValueDataType())
    {
        NewInstance.InitializeAs(StateMetadata->GetValueDataType());
        // 这里可以添加从元数据初始化结构体的逻辑
    }
    return NewInstance;
}

void FSyStateParameterSetCustomization::SerializeParameterStruct(const FInstancedStruct& Instance, const TSharedRef<IPropertyHandle>& Handle)
{
    if (void* ElementData = nullptr; Handle->GetValueData(ElementData) == FPropertyAccess::Success)
    {
        *(static_cast<FInstancedStruct*>(ElementData)) = Instance;
        Handle->NotifyPostChange(EPropertyChangeType::ValueSet);
    }
}

void FSyStateParameterSetCustomization::UpdateParameterStructForTag(const FGameplayTag& Tag, const TSharedRef<IPropertyHandle>& ParamsPropertyHandle)
{
    // 获取参数数组句柄
    TSharedPtr<IPropertyHandle> ArrayHandle = ParamsPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSyStateParams, Params));
    if (!ArrayHandle.IsValid() || !ArrayHandle->IsValidHandle())
    {
        UE_LOG(LogTemp, Warning, TEXT("FSyStateParameterSetCustomization: Could not get 'Params' array handle."));
        return;
    }

    TSharedRef<IPropertyHandleArray> ArrayHandleRef = ArrayHandle->AsArray().ToSharedRef();
    ArrayHandleRef->EmptyArray();

    if (Tag.IsValid())
    {
        // 获取所有与标签关联的元数据
        TArray<UO_TagMetadata*> AllMetadata = UDS_TagMetadata::GetTagMetadata(Tag);
        TArray<USyStateMetadataBase*> StateMetadataList;

        // 收集所有状态元数据
        for (UO_TagMetadata* Metadata : AllMetadata)
        {
            if (USyStateMetadataBase* StateMetadata = Cast<USyStateMetadataBase>(Metadata))
            {
                StateMetadataList.Add(StateMetadata);
            }
        }

        // 为每个元数据创建对应的参数
        for (USyStateMetadataBase* StateMetadata : StateMetadataList)
        {
            if (StateMetadata && StateMetadata->GetValueDataType())
            {
                // 添加新元素
                ArrayHandleRef->AddItem();
                uint32 NumElements;
                ArrayHandleRef->GetNumElements(NumElements);
                
                if (NumElements > 0)
                {
                    TSharedPtr<IPropertyHandle> NewElementHandle = ArrayHandleRef->GetElement(NumElements - 1);
                    if (NewElementHandle.IsValid())
                    {
                        // 创建并设置新的实例化结构体
                        FInstancedStruct NewInstance = CreateParameterStructFromMetadata(StateMetadata);
                        SerializeParameterStruct(NewInstance, NewElementHandle.ToSharedRef());
                    }
                }
            }
        }
    }
}

#undef LOCTEXT_NAMESPACE 
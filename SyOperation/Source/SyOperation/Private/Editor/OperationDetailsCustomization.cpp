// Copyright Epic Games, Inc. All Rights Reserved.

#include "Editor/OperationDetailsCustomization.h"
#include "Editor/OperationEditorUtils.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailPropertyRow.h"
#include "PropertyHandle.h"
#include "GameplayTagContainer.h"
#include "OperationTypes.h"
#include "PropertyEditorModule.h"
#include "IPropertyRowGenerator.h"
#include "UObject/UnrealType.h"
#include "UObject/Class.h"

TSharedRef<IDetailCustomization> FSyOperationDetailsCustomization::MakeInstance()
{
    return MakeShared<FSyOperationDetailsCustomization>();
}

void FSyOperationDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    // 获取当前编辑的对象
    TArray<TWeakObjectPtr<UObject>> Objects;
    DetailBuilder.GetObjectsBeingCustomized(Objects);
    if (Objects.Num() > 0)
    {
        EditedObject = Objects[0];
    }

    // 自定义Source部分
    IDetailCategoryBuilder& SourceCategory = DetailBuilder.EditCategory("Source");
    SourceTypeTagHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(FSyOperation, Source.SourceTypeTag));
    TSharedPtr<IPropertyHandle> SourceParametersHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(FSyOperation, Source.Parameters));

    if (SourceTypeTagHandle.IsValid() && SourceParametersHandle.IsValid())
    {
        SourceTypeTagHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FSyOperationDetailsCustomization::OnSourceTagChanged));
    }

    // 自定义Target部分
    IDetailCategoryBuilder& TargetCategory = DetailBuilder.EditCategory("Target");
    TargetTypeTagHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(FSyOperation, Target.TargetTypeTag));
    TSharedPtr<IPropertyHandle> TargetParametersHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(FSyOperation, Target.Parameters));

    if (TargetTypeTagHandle.IsValid() && TargetParametersHandle.IsValid())
    {
        TargetTypeTagHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FSyOperationDetailsCustomization::OnTargetTagChanged));
    }
}

void FSyOperationDetailsCustomization::OnSourceTagChanged()
{
    if (FSyOperation* Operation = GetOperationBeingEdited())
    {
        FGameplayTag CurrentTag;
        if (SourceTypeTagHandle.IsValid())
        {
            FString TagString;
            if (SourceTypeTagHandle->GetValue(TagString) == FPropertyAccess::Success)
            {
                CurrentTag = FGameplayTag::RequestGameplayTag(FName(*TagString));
            }
        }
        
        USyOperationEditorUtils::UpdateInstancedStructTypeForTag(Operation->Source.Parameters, CurrentTag);
    }
}


void FSyOperationDetailsCustomization::OnTargetTagChanged()
{
    if (FSyOperation* Operation = GetOperationBeingEdited())
    {
        FGameplayTag CurrentTag;
        if (TargetTypeTagHandle.IsValid())
        {
            FString TagString;
            if (TargetTypeTagHandle->GetValue(TagString) == FPropertyAccess::Success)
            {
                CurrentTag = FGameplayTag::RequestGameplayTag(FName(*TagString));
            }
        }
        
        USyOperationEditorUtils::UpdateInstancedStructTypeForTag(Operation->Target.Parameters, CurrentTag);
    }
}

FSyOperation* FSyOperationDetailsCustomization::GetOperationBeingEdited() const
{
    if (EditedObject.IsValid())
    {
        UObject* Object = EditedObject.Get();
        if (Object)
        {
            // 获取FSyOperation属性
            FStructProperty* StructProperty = FindFProperty<FStructProperty>(Object->GetClass(), TEXT("Operation"));
            if (StructProperty && StructProperty->Struct == FSyOperation::StaticStruct())
            {
                return StructProperty->ContainerPtrToValuePtr<FSyOperation>(Object);
            }
        }
    }

    return nullptr;
} 
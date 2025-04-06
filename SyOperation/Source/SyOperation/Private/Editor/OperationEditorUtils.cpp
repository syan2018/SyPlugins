// Copyright Epic Games, Inc. All Rights Reserved.

#include "Editor/OperationEditorUtils.h"
#include "OperationParamSchemaMetadata.h"
#include "TagMetadata/Public/O_TagMetadataCollection.h"
#include "TagMetadata/Public/DS_TagMetadata.h"

UScriptStruct* USyOperationEditorUtils::GetParameterStructTypeForTag(const FGameplayTag& Tag)
{
    // 使用UDS_TagMetadata获取与Tag关联的USyOperationParamSchemaMetadata
    if (USyOperationParamSchemaMetadata* SchemaMetadata = Cast<USyOperationParamSchemaMetadata>(
        UDS_TagMetadata::GetTagMetadataByClass(Tag, USyOperationParamSchemaMetadata::StaticClass())))
    {
        return SchemaMetadata->GetParameterStructType();
    }

    return nullptr;
}

bool USyOperationEditorUtils::UpdateInstancedStructTypeForTag(FSyInstancedStruct& InstancedStruct, const FGameplayTag& Tag)
{
    UScriptStruct* ExpectedStructType = GetParameterStructTypeForTag(Tag);
    if (!ExpectedStructType)
    {
        return false;
    }

    // 如果当前类型不匹配，则更新类型
    if (InstancedStruct.GetScriptStruct() != ExpectedStructType)
    {
        InstancedStruct.InitializeAs(ExpectedStructType);
        return true;
    }

    return true;
}

bool USyOperationEditorUtils::ValidateInstancedStructTypeForTag(const FSyInstancedStruct& InstancedStruct, const FGameplayTag& Tag)
{
    UScriptStruct* ExpectedStructType = GetParameterStructTypeForTag(Tag);
    if (!ExpectedStructType)
    {
        return false;
    }

    return InstancedStruct.GetScriptStruct() == ExpectedStructType;
} 
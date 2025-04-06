// Copyright Epic Games, Inc. All Rights Reserved.

#include "StateMetadataTypes.h"
#include "SyCore/Public/Foundation/Utilities/SyInstancedStruct.h"
#include "DS_TagMetadata.h"

// USyStateMetadataBase
USyStateMetadataBase::USyStateMetadataBase()
{
    // 默认构造函数
}

// USyTagStateMetadata
USyTagStateMetadata::USyTagStateMetadata()
{
    // 默认构造函数
}

void USyTagStateMetadata::InitializeFromParams(const FSyInstancedStruct& InitParams)
{
    // 使用类型安全的参数访问
    if (const FGameplayTag* TagValue = InitParams.GetPtr<FGameplayTag>())
    {
        Value = *TagValue;
    }
}

void USyTagStateMetadata::ApplyModification(const FSyInstancedStruct& ModificationParams)
{
    // 使用类型安全的参数访问
    if (const FGameplayTag* TagValue = ModificationParams.GetPtr<FGameplayTag>())
    {
        Value = *TagValue;
    }
} 
// Copyright Epic Games, Inc. All Rights Reserved.

#include "StateMetadataTypes.h"
#include "SyCore/Public/Foundation/Utilities/SyInstancedStruct.h"

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
    if (const FGameplayTag* TagValue = InitParams.GetPtr<FGameplayTag>())
    {
        Value = *TagValue;
    }
}

void USyTagStateMetadata::ApplyModification(const FSyInstancedStruct& ModificationParams)
{
    if (const FGameplayTag* TagValue = ModificationParams.GetPtr<FGameplayTag>())
    {
        Value = *TagValue;
    }
} 
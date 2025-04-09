// Copyright Epic Games, Inc. All Rights Reserved.

#include "StateMetadataTypes.h"
#include "SyCore/Public/Foundation/Utilities/SyInstancedStruct.h"
#include "Logging/LogMacros.h"

// Define a log category if you don't have one already
// DECLARE_LOG_CATEGORY_EXTERN(LogSyStateCore, Log, All);
// DEFINE_LOG_CATEGORY(LogSyStateCore);

// USyStateMetadataBase
USyStateMetadataBase::USyStateMetadataBase()
{
    // 默认构造函数
}

void USyStateMetadataBase::InitializeFromParams(const FSyInstancedStruct& InitParams)
{
    ValidateAndProcessParams(InitParams, TEXT("InitializeFromParams"));
}

void USyStateMetadataBase::ApplyModification(const FSyInstancedStruct& ModificationParams)
{
    ValidateAndProcessParams(ModificationParams, TEXT("ApplyModification"));
}

bool USyStateMetadataBase::ValidateAndProcessParams(const FSyInstancedStruct& Params, const TCHAR* OperationName)
{
    if (!Params.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("%s failed: Invalid params for %s"), OperationName, *GetNameSafe(this));
        return false;
    }

    if (Params.GetScriptStruct() != GetValueDataType())
    {
        UE_LOG(LogTemp, Error, TEXT("%s failed for %s: Type mismatch. Expected %s, Got %s"),
            OperationName,
            *GetNameSafe(this),
            *GetNameSafe(GetValueDataType()),
            *GetNameSafe(Params.GetScriptStruct()));
        return false;
    }

    UpdateValue(Params);
    return true;
}

// USyTagStateMetadata
USyTagStateMetadata::USyTagStateMetadata()
{
    // 默认构造函数
}

UScriptStruct* USyTagStateMetadata::GetValueDataType() const
{
    return FGameplayTag::StaticStruct();
}

void USyTagStateMetadata::UpdateValue(const FSyInstancedStruct& Params)
{
    if (const FGameplayTag* TagValue = Params.GetPtr<FGameplayTag>())
    {
        Value = *TagValue;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UpdateValue failed to get pointer for %s despite matching types: Expected %s, Got %s"),
            *GetNameSafe(this),
            *GetNameSafe(GetValueDataType()),
            *GetNameSafe(Params.GetScriptStruct()));
    }
} 
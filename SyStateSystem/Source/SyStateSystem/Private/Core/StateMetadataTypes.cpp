// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/StateMetadataTypes.h"
#include "Logging/LogMacros.h"
#include "StructUtils/InstancedStruct.h"

USyStateMetadataBase::USyStateMetadataBase()
{
}

void USyStateMetadataBase::InitializeFromParams(const FInstancedStruct& InitParams)
{
    ValidateAndProcessParams(InitParams, TEXT("InitializeFromParams"));
}

void USyStateMetadataBase::ApplyModification(const FInstancedStruct& ModificationParams)
{
    ValidateAndProcessParams(ModificationParams, TEXT("ApplyModification"));
}

void USyStateMetadataBase::SetValueStruct_Implementation(const FInstancedStruct& InValue)
{
    if (InValue.IsValid() && InValue.GetScriptStruct() == GetValueDataType())
    {
        // 子类应该重载此方法以处理具体的值设置
        UE_LOG(LogTemp, Warning, TEXT("SetValueStruct called on base class %s, which doesn't implement value setting"), *GetNameSafe(this));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SetValueStruct failed: Type mismatch for %s"), *GetNameSafe(this));
    }
}

bool USyStateMetadataBase::ValidateAndProcessParams(const FInstancedStruct& Params, const TCHAR* OperationName)
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

    SetValueStruct(Params);
    return true;
}
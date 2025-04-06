// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SyCore/Public/Foundation/Utilities/SyInstancedStruct.h"
#include "OperationEditorUtils.generated.h"

/**
 * USyOperationEditorUtils - 操作编辑器工具类
 * 
 * 提供编辑器中的工具函数，用于改善Tag与FInstancedStruct参数类型的联动体验
 */
UCLASS()
class SYOPERATION_API USyOperationEditorUtils : public UObject
{
    GENERATED_BODY()

public:
    /** 根据Tag获取参数结构体类型 */
    UFUNCTION(BlueprintCallable, Category = "SyOperation|Editor")
    static UScriptStruct* GetParameterStructTypeForTag(const FGameplayTag& Tag);

    /** 根据Tag更新FInstancedStruct的类型 */
    UFUNCTION(BlueprintCallable, Category = "SyOperation|Editor")
    static bool UpdateInstancedStructTypeForTag(FSyInstancedStruct& InstancedStruct, const FGameplayTag& Tag);

    /** 检查FInstancedStruct的类型是否与Tag匹配 */
    UFUNCTION(BlueprintCallable, Category = "SyOperation|Editor")
    static bool ValidateInstancedStructTypeForTag(const FSyInstancedStruct& InstancedStruct, const FGameplayTag& Tag);
}; 
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "O_TagMetadata.h"
#include "OperationParamSchemaMetadata.generated.h"

/**
 * USyOperationParamSchemaMetadata - 操作参数Schema元数据类
 * 
 * 用于通过TagMetadata插件将GameplayTag与期望的参数结构体类型关联起来
 * 在TagMetadataCollection中配置后，可以通过Tag查询获取对应的参数结构体类型
 */
UCLASS(Blueprintable)
class SYOPERATION_API USyOperationParamSchemaMetadata : public UO_TagMetadata
{
    GENERATED_BODY()

public:
    /** 构造函数 */
    USyOperationParamSchemaMetadata();

    /** 获取参数结构体类型 */
    UFUNCTION(BlueprintPure, Category = "SyOperation|Schema")
    UScriptStruct* GetParameterStructType() const { return ParameterStructType; }

    /** 设置参数结构体类型 */
    UFUNCTION(BlueprintCallable, Category = "SyOperation|Schema")
    void SetParameterStructType(UScriptStruct* NewType) { ParameterStructType = NewType; }

protected:
    /** 参数结构体类型 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schema", meta = (AllowAbstract = "false", MetaClass = "/Script/CoreUObject.ScriptStruct"))
    UScriptStruct* ParameterStructType;
}; 
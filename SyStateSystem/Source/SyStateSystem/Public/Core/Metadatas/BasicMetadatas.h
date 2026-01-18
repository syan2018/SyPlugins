#pragma once

#include "Core/StateMetadataTypes.h"
#include "GameplayTagContainer.h"
#include "BasicMetadataValueTypes.h" // <<< 包含基础类型 USTRUCT 定义
#include "BasicMetadatas.generated.h"

//----------------------------------------------------------------------
// Bool Metadata
//----------------------------------------------------------------------

/**
 * USyBoolMetadata - 布尔值状态元数据
 */
UCLASS(Blueprintable)
class SYSTATESYSTEM_API USyBoolMetadata : public USyStateMetadataBase
{
    GENERATED_BODY()

public:
    /** 返回此元数据管理的具体数据类型 (FSyBoolValue) */
    virtual UScriptStruct* GetValueDataType_Implementation() const override { return FSyBoolValue::StaticStruct(); }

    /** 获取存储的值 */
    virtual FInstancedStruct GetValueStruct_Implementation() const override
    {
        FInstancedStruct ValueStruct;
        ValueStruct.InitializeAs<FSyBoolValue>(FSyBoolValue(Value));
        return ValueStruct;
    }

    /** 设置存储的值 */
    virtual void SetValueStruct_Implementation(const FInstancedStruct& InValue) override
    {
        if (InValue.IsValid() && InValue.GetScriptStruct() == FSyBoolValue::StaticStruct())
        {
            if (const FSyBoolValue* ValuePtr = InValue.GetPtr<FSyBoolValue>())
            {
                Value = ValuePtr->Value;
            }
        }
    }

    /** 获取布尔值 */
    UFUNCTION(BlueprintPure, Category = "SyStateCore|BoolMetadata")
    bool GetValue() const { return Value; }

    /** 设置布尔值 */
    UFUNCTION(BlueprintCallable, Category = "SyStateCore|BoolMetadata")
    void SetValue(bool InValue) { Value = InValue; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyStateCore|BoolMetadata")
    bool Value = false;
};

//----------------------------------------------------------------------
// Int Metadata
//----------------------------------------------------------------------

/**
 * USyIntMetadata - 整型值状态元数据
 */
UCLASS(Blueprintable)
class SYSTATESYSTEM_API USyIntMetadata : public USyStateMetadataBase
{
    GENERATED_BODY()

public:
    virtual UScriptStruct* GetValueDataType_Implementation() const override { return FSyIntValue::StaticStruct(); }

    virtual FInstancedStruct GetValueStruct_Implementation() const override
    {
        FInstancedStruct ValueStruct;
        ValueStruct.InitializeAs<FSyIntValue>(FSyIntValue(Value));
        return ValueStruct;
    }

    virtual void SetValueStruct_Implementation(const FInstancedStruct& InValue) override
    {
        if (InValue.IsValid() && InValue.GetScriptStruct() == FSyIntValue::StaticStruct())
        {
            if (const FSyIntValue* ValuePtr = InValue.GetPtr<FSyIntValue>())
            {
                Value = ValuePtr->Value;
            }
        }
    }

    UFUNCTION(BlueprintPure, Category = "SyStateCore|IntMetadata")
    int32 GetValue() const { return Value; }

    UFUNCTION(BlueprintCallable, Category = "SyStateCore|IntMetadata")
    void SetValue(int32 InValue) { Value = InValue; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyStateCore|IntMetadata")
    int32 Value = 0;
};

//----------------------------------------------------------------------
// Float Metadata
//----------------------------------------------------------------------

/**
 * USyFloatMetadata - 浮点值状态元数据
 */
UCLASS(Blueprintable)
class SYSTATESYSTEM_API USyFloatMetadata : public USyStateMetadataBase
{
    GENERATED_BODY()

public:
    virtual UScriptStruct* GetValueDataType_Implementation() const override { return FSyFloatValue::StaticStruct(); }

    virtual FInstancedStruct GetValueStruct_Implementation() const override
    {
        FInstancedStruct ValueStruct;
        ValueStruct.InitializeAs<FSyFloatValue>(FSyFloatValue(Value));
        return ValueStruct;
    }

    virtual void SetValueStruct_Implementation(const FInstancedStruct& InValue) override
    {
        if (InValue.IsValid() && InValue.GetScriptStruct() == FSyFloatValue::StaticStruct())
        {
            if (const FSyFloatValue* ValuePtr = InValue.GetPtr<FSyFloatValue>())
            {
                Value = ValuePtr->Value;
            }
        }
    }

    UFUNCTION(BlueprintPure, Category = "SyStateCore|FloatMetadata")
    float GetValue() const { return Value; }

    UFUNCTION(BlueprintCallable, Category = "SyStateCore|FloatMetadata")
    void SetValue(float InValue) { Value = InValue; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyStateCore|FloatMetadata")
    float Value = 0.0f;
};

//----------------------------------------------------------------------
// String Metadata
//----------------------------------------------------------------------

/**
 * USyStringMetadata - 字符串状态元数据
 */
UCLASS(Blueprintable)
class SYSTATESYSTEM_API USyStringMetadata : public USyStateMetadataBase
{
    GENERATED_BODY()

public:
    virtual UScriptStruct* GetValueDataType_Implementation() const override { return FSyStringValue::StaticStruct(); }

    virtual FInstancedStruct GetValueStruct_Implementation() const override
    {
        FInstancedStruct ValueStruct;
        ValueStruct.InitializeAs<FSyStringValue>(FSyStringValue(Value));
        return ValueStruct;
    }

    virtual void SetValueStruct_Implementation(const FInstancedStruct& InValue) override
    {
        if (InValue.IsValid() && InValue.GetScriptStruct() == FSyStringValue::StaticStruct())
        {
            if (const FSyStringValue* ValuePtr = InValue.GetPtr<FSyStringValue>())
            {
                Value = ValuePtr->Value;
            }
        }
    }

    UFUNCTION(BlueprintPure, Category = "SyStateCore|StringMetadata")
    const FString& GetValue() const { return Value; }

    UFUNCTION(BlueprintCallable, Category = "SyStateCore|StringMetadata")
    void SetValue(const FString& InValue) { Value = InValue; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyStateCore|StringMetadata")
    FString Value;
};

//----------------------------------------------------------------------
// Name Metadata
//----------------------------------------------------------------------

/**
 * USyNameMetadata - FName 状态元数据
 */
UCLASS(Blueprintable)
class SYSTATESYSTEM_API USyNameMetadata : public USyStateMetadataBase
{
    GENERATED_BODY()

public:
    virtual UScriptStruct* GetValueDataType_Implementation() const override { return FSyNameValue::StaticStruct(); }

    virtual FInstancedStruct GetValueStruct_Implementation() const override
    {
        FInstancedStruct ValueStruct;
        ValueStruct.InitializeAs<FSyNameValue>(FSyNameValue(Value));
        return ValueStruct;
    }

    virtual void SetValueStruct_Implementation(const FInstancedStruct& InValue) override
    {
        if (InValue.IsValid() && InValue.GetScriptStruct() == FSyNameValue::StaticStruct())
        {
            if (const FSyNameValue* ValuePtr = InValue.GetPtr<FSyNameValue>())
            {
                Value = ValuePtr->Value;
            }
        }
    }

    UFUNCTION(BlueprintPure, Category = "SyStateCore|NameMetadata")
    FName GetValue() const { return Value; }

    UFUNCTION(BlueprintCallable, Category = "SyStateCore|NameMetadata")
    void SetValue(const FName& InValue) { Value = InValue; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyStateCore|NameMetadata")
    FName Value = NAME_None;
};

//----------------------------------------------------------------------
// Vector Metadata
//----------------------------------------------------------------------

/**
 * USyVectorMetadata - FVector 状态元数据
 */
UCLASS(Blueprintable)
class SYSTATESYSTEM_API USyVectorMetadata : public USyStateMetadataBase
{
    GENERATED_BODY()

public:
    virtual UScriptStruct* GetValueDataType_Implementation() const override { return FSyVectorValue::StaticStruct(); }

    virtual FInstancedStruct GetValueStruct_Implementation() const override
    {
        FInstancedStruct ValueStruct;
        ValueStruct.InitializeAs<FSyVectorValue>(FSyVectorValue(Value));
        return ValueStruct;
    }

    virtual void SetValueStruct_Implementation(const FInstancedStruct& InValue) override
    {
        if (InValue.IsValid() && InValue.GetScriptStruct() == FSyVectorValue::StaticStruct())
        {
            if (const FSyVectorValue* ValuePtr = InValue.GetPtr<FSyVectorValue>())
            {
                Value = ValuePtr->Value;
            }
        }
    }

    UFUNCTION(BlueprintPure, Category = "SyStateCore|VectorMetadata")
    FVector GetValue() const { return Value; }

    UFUNCTION(BlueprintCallable, Category = "SyStateCore|VectorMetadata")
    void SetValue(const FVector& InValue) { Value = InValue; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyStateCore|VectorMetadata")
    FVector Value = FVector::ZeroVector;
};

//----------------------------------------------------------------------
// Rotator Metadata
//----------------------------------------------------------------------

/**
 * USyRotatorMetadata - FRotator 状态元数据
 */
UCLASS(Blueprintable)
class SYSTATESYSTEM_API USyRotatorMetadata : public USyStateMetadataBase
{
    GENERATED_BODY()

public:
    virtual UScriptStruct* GetValueDataType_Implementation() const override { return FSyRotatorValue::StaticStruct(); }

    virtual FInstancedStruct GetValueStruct_Implementation() const override
    {
        FInstancedStruct ValueStruct;
        ValueStruct.InitializeAs<FSyRotatorValue>(FSyRotatorValue(Value));
        return ValueStruct;
    }

    virtual void SetValueStruct_Implementation(const FInstancedStruct& InValue) override
    {
        if (InValue.IsValid() && InValue.GetScriptStruct() == FSyRotatorValue::StaticStruct())
        {
            if (const FSyRotatorValue* ValuePtr = InValue.GetPtr<FSyRotatorValue>())
            {
                Value = ValuePtr->Value;
            }
        }
    }

    UFUNCTION(BlueprintPure, Category = "SyStateCore|RotatorMetadata")
    FRotator GetValue() const { return Value; }

    UFUNCTION(BlueprintCallable, Category = "SyStateCore|RotatorMetadata")
    void SetValue(const FRotator& InValue) { Value = InValue; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyStateCore|RotatorMetadata")
    FRotator Value = FRotator::ZeroRotator;
};

//----------------------------------------------------------------------
// Transform Metadata
//----------------------------------------------------------------------

/**
 * USyTransformMetadata - FTransform 状态元数据
 */
UCLASS(Blueprintable)
class SYSTATESYSTEM_API USyTransformMetadata : public USyStateMetadataBase
{
    GENERATED_BODY()

public:
    virtual UScriptStruct* GetValueDataType_Implementation() const override { return FSyTransformValue::StaticStruct(); }

    virtual FInstancedStruct GetValueStruct_Implementation() const override
    {
        FInstancedStruct ValueStruct;
        ValueStruct.InitializeAs<FSyTransformValue>(FSyTransformValue(Value));
        return ValueStruct;
    }

    virtual void SetValueStruct_Implementation(const FInstancedStruct& InValue) override
    {
        if (InValue.IsValid() && InValue.GetScriptStruct() == FSyTransformValue::StaticStruct())
        {
            if (const FSyTransformValue* ValuePtr = InValue.GetPtr<FSyTransformValue>())
            {
                Value = ValuePtr->Value;
            }
        }
    }

    UFUNCTION(BlueprintPure, Category = "SyStateCore|TransformMetadata")
    const FTransform& GetValue() const { return Value; }

    UFUNCTION(BlueprintCallable, Category = "SyStateCore|TransformMetadata")
    void SetValue(const FTransform& InValue) { Value = InValue; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyStateCore|TransformMetadata")
    FTransform Value = FTransform::Identity;
};

UCLASS(Blueprintable)
class SYSTATESYSTEM_API USyTagMetadata : public USyStateMetadataBase
{
    GENERATED_BODY()

public:
    virtual UScriptStruct* GetValueDataType_Implementation() const override {return FGameplayTag::StaticStruct();}
    virtual FInstancedStruct GetValueStruct_Implementation() const override
    {
        FInstancedStruct Result;
        Result.InitializeAs<FGameplayTag>(Value);
        return Result;
    }
    virtual void SetValueStruct_Implementation(const FInstancedStruct& InValue) override{
        if (InValue.IsValid() && InValue.GetScriptStruct() == FGameplayTag::StaticStruct())
        {
            if (const FGameplayTag* TagValue = InValue.GetPtr<FGameplayTag>())
            {
                Value = *TagValue;
            }
        }
    }

    UFUNCTION(BlueprintPure, Category = "SyStateCore|TagStateMetadata")
    FGameplayTag GetValue() const { return Value; }

    UFUNCTION(BlueprintCallable, Category = "SyStateCore|TagStateMetadata")
    void SetValue(const FGameplayTag& InValue) { Value = InValue; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SyStateCore|TagStateMetadata")
    FGameplayTag Value;
};

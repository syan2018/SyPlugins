#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/NameTypes.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"
#include "Math/Transform.h"
#include "Foundation/Utilities/SyInstancedStruct.h"
#include "BasicMetadataValueTypes.generated.h"

// --- USTRUCT Wrappers for Basic Types ---

USTRUCT(BlueprintType)
struct SYSTATECORE_API FSyBoolValue: public FSyBaseInstancedStruct
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Value")
    bool Value = false;

    FSyBoolValue(bool InValue = false) : Value(InValue) {}
    // FSyBoolValue() = default;
};

USTRUCT(BlueprintType)
struct SYSTATECORE_API FSyIntValue: public FSyBaseInstancedStruct
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Value")
    int32 Value = 0;

    FSyIntValue(int32 InValue = 0) : Value(InValue) {}
    // FSyIntValue() = default;
};

USTRUCT(BlueprintType)
struct SYSTATECORE_API FSyFloatValue: public FSyBaseInstancedStruct
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Value")
    float Value = 0.0f;

    FSyFloatValue(float InValue = 0.0f) : Value(InValue) {}
    // FSyFloatValue() = default;
};

USTRUCT(BlueprintType)
struct SYSTATECORE_API FSyStringValue: public FSyBaseInstancedStruct
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Value")
    FString Value;

    FSyStringValue(const FString& InValue = TEXT("")) : Value(InValue) {}
    // FSyStringValue() = default;
};

USTRUCT(BlueprintType)
struct SYSTATECORE_API FSyNameValue: public FSyBaseInstancedStruct
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Value")
    FName Value = NAME_None;

    FSyNameValue(const FName& InValue = NAME_None) : Value(InValue) {}
    // FSyNameValue() = default;
};

USTRUCT(BlueprintType)
struct SYSTATECORE_API FSyVectorValue: public FSyBaseInstancedStruct
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Value")
    FVector Value = FVector::ZeroVector;

    FSyVectorValue(const FVector& InValue = FVector::ZeroVector) : Value(InValue) {}
    // FSyVectorValue() = default;
};

USTRUCT(BlueprintType)
struct SYSTATECORE_API FSyRotatorValue: public FSyBaseInstancedStruct
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Value")
    FRotator Value = FRotator::ZeroRotator;

    FSyRotatorValue(const FRotator& InValue = FRotator::ZeroRotator) : Value(InValue) {}
    // FSyRotatorValue() = default;
};

USTRUCT(BlueprintType)
struct SYSTATECORE_API FSyTransformValue: public FSyBaseInstancedStruct
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Value")
    FTransform Value = FTransform::Identity;

    FSyTransformValue(const FTransform& InValue = FTransform::Identity) : Value(InValue) {}
    // FSyTransformValue() = default;
};

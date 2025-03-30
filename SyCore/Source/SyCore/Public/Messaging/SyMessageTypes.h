#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SyMessageTypes.generated.h"

// 消息源结构
USTRUCT(BlueprintType)
struct SYCORE_API FSyMessageSource
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag SourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid SourceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SourceAlias;
};

// 消息内容结构
USTRUCT(BlueprintType)
struct SYCORE_API FSyMessageContent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag MessageType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FString> Metadata;
};

// 完整消息结构
USTRUCT(BlueprintType)
struct SYCORE_API FSyMessage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FSyMessageSource Source;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FSyMessageContent Content;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Timestamp;
}; 
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SyMessageTypes.h"
#include "SyMessageReceiver.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class USyMessageReceiver : public UInterface
{
    GENERATED_BODY()
};

class SYCORE_API ISyMessageReceiver
{
    GENERATED_BODY()

public:
    // 消息接收接口
    UFUNCTION(BlueprintNativeEvent, Category = "SyMessage")
    void OnMessageReceived(const FSyMessage& Message);
}; 
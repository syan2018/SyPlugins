#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Entity/SyIdentityComponent.h"
#include "SyMessageTypes.h"
#include "Foundation/ISyComponentInterface.h"
#include "SyMessageComponent.generated.h"

/**
 * 消息组件 - 负责实体的消息分发
 * 1. 依赖Identity组件获取实体标识
 * 2. 提供消息发送接口
 * 3. 不处理消息接收逻辑
 */
UCLASS(Blueprintable, ClassGroup=(SyCore), meta=(BlueprintSpawnableComponent))
class SYCORE_API USyMessageComponent : public UActorComponent, public ISyComponentInterface
{
    GENERATED_BODY()

public:
    USyMessageComponent();

    // 实现ISyComponentInterface接口
    virtual FName GetComponentType() const override { return TEXT("Message"); }

    // 发送消息接口（带Metadata）
    UFUNCTION(BlueprintCallable, Category = "SyMessage")
    bool SendMessageWithMetadata(const FGameplayTag& MessageType, const TMap<FName, FString>& Metadata);

    // 发送消息接口（不带Metadata）
    UFUNCTION(BlueprintCallable, Category = "SyMessage")
    bool SendMessage(const FGameplayTag& MessageType);

protected:
    virtual void BeginPlay() override;
    virtual void OnComponentCreated() override;

private:
    // 依赖的Identity组件
    UPROPERTY()
    TObjectPtr<USyIdentityComponent> IdentityComponent;

    // 获取消息总线
    class USyMessageBus* GetMessageBus() const;

    // 内部消息发送实现
    bool SendMessageInternal(const FGameplayTag& MessageType, const TMap<FName, FString>& Metadata);
};

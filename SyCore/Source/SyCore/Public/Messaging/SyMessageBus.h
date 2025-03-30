#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SyMessageTypes.h"
#include "SyMessageReceiver.h"
#include "SyMessageFilter.h"
#include "SyMessageBus.generated.h"

/**
 * 消息总线 - 负责消息的分发和订阅管理
 * 1. 管理全局消息订阅
 * 2. 处理消息分发
 * 3. 提供Flow节点所需的订阅接口
 */
UCLASS()
class SYCORE_API USyMessageBus : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // 广播消息
    void BroadcastMessage(const FSyMessage& Message);

    // Flow节点订阅接口
    void SubscribeWithFilter(USyMessageFilterComposer* Filter, UObject* Subscriber);
    void UnsubscribeWithFilter(USyMessageFilterComposer* Filter, UObject* Subscriber);

    // 获取订阅者列表
    TArray<UObject*> GetSubscribersForFilter(USyMessageFilterComposer* Filter) const;

private:
    // 消息订阅表
    TMultiMap<USyMessageFilterComposer*, UObject*> MessageSubscriptions;

    // 消息过滤和分发
    void DispatchMessage(const FSyMessage& Message);
}; 
#include "Messaging/SyMessageComponent.h"
#include "Messaging/SyMessageBus.h"
#include "Identity/SyIdentityComponent.h"

USyMessageComponent::USyMessageComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void USyMessageComponent::OnComponentCreated()
{
    Super::OnComponentCreated();
    EnsureRequiredComponents();
}

void USyMessageComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // 获取Identity组件
    IdentityComponent = GetOwner()->FindComponentByClass<USyIdentityComponent>();
}

void USyMessageComponent::EnsureRequiredComponents()
{
    // 确保Identity组件存在
    if (!GetOwner()->FindComponentByClass<USyIdentityComponent>())
    {
        // 创建Identity组件
        USyIdentityComponent* NewIdentityComponent = NewObject<USyIdentityComponent>(GetOwner());
        NewIdentityComponent->RegisterComponent();
    }
}

bool USyMessageComponent::SendMessage(const FGameplayTag& MessageType)
{
    return SendMessageInternal(MessageType, TMap<FName, FString>());
}

bool USyMessageComponent::SendMessageWithMetadata(const FGameplayTag& MessageType, const TMap<FName, FString>& Metadata)
{
    return SendMessageInternal(MessageType, Metadata);
}

bool USyMessageComponent::SendMessageInternal(const FGameplayTag& MessageType, const TMap<FName, FString>& Metadata)
{
    if (!IdentityComponent || !IdentityComponent->HasValidId())
    {
        return false;
    }

    // 构建消息
    FSyMessage Message;
    Message.Source.SourceId = IdentityComponent->GetEntityId();
    Message.Source.SourceType = IdentityComponent->GetEntityTags().First();
    Message.Content.MessageType = MessageType;
    Message.Content.Metadata = Metadata;
    Message.Timestamp = FDateTime::Now();

    // 通过消息总线广播
    if (USyMessageBus* MessageBus = GetMessageBus())
    {
        MessageBus->BroadcastMessage(Message);
        return true;
    }

    return false;
}

USyMessageBus* USyMessageComponent::GetMessageBus() const
{
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        return GameInstance->GetSubsystem<USyMessageBus>();
    }
    return nullptr;
} 
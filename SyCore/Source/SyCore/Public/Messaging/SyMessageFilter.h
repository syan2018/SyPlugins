#pragma once

#include "CoreMinimal.h"
#include "SyMessageTypes.h"
#include "SyMessageFilter.generated.h"

/**
 * 消息过滤规则基类
 */
UCLASS(Abstract)
class SYCORE_API USyMessageFilter : public UObject
{
    GENERATED_BODY()

public:
    virtual bool Matches(const FSyMessage& Message) const { return false; }
};

/**
 * 消息源类型过滤规则
 */
UCLASS()
class SYCORE_API USySourceTypeFilter : public USyMessageFilter
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag SourceType;

    virtual bool Matches(const FSyMessage& Message) const override;
};

/**
 * 消息源GUID过滤规则
 */
UCLASS()
class SYCORE_API USySourceGuidFilter : public USyMessageFilter
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid SourceGuid;

    virtual bool Matches(const FSyMessage& Message) const override;
};

/**
 * 消息源别名过滤规则
 */
UCLASS()
class SYCORE_API USySourceAliasFilter : public USyMessageFilter
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SourceAlias;

    virtual bool Matches(const FSyMessage& Message) const override;
};

/**
 * 消息类型过滤规则
 */
UCLASS()
class SYCORE_API USyMessageTypeFilter : public USyMessageFilter
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag MessageType;

    virtual bool Matches(const FSyMessage& Message) const override;
};

/**
 * 消息过滤器组合器
 */
UCLASS()
class SYCORE_API USyMessageFilterComposer : public UObject
{
    GENERATED_BODY()

public:
    // 添加过滤规则
    void AddFilter(USyMessageFilter* Filter);
    
    // 清除所有过滤规则
    void ClearFilters();
    
    // 检查消息是否匹配所有过滤规则
    bool Matches(const FSyMessage& Message) const;

private:
    UPROPERTY()
    TArray<TObjectPtr<USyMessageFilter>> Filters;
};

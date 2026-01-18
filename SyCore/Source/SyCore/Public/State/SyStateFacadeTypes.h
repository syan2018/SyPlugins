#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "SyStateFacadeTypes.generated.h"

/**
 * ESyStateScope - 状态写入/路由的目标范围
 *
 * - Entity: 作用于单个实体（推荐用于战斗、阶段、死亡等）
 * - Type:   作用于某一类实体（兼容既有 StateManager TargetTypeTag 快照语义）
 * - World:  全局状态（任务/关卡/系统开关等）
 */
UENUM(BlueprintType)
enum class ESyStateScope : uint8
{
	Entity UMETA(DisplayName = "Entity"),
	Type UMETA(DisplayName = "Type"),
	World UMETA(DisplayName = "World"),
};

/**
 * ESyStateWriteLayer - 状态写入层级（对应“临时/持久”两种典型需求）
 *
 * - Temporary: 本地临时层（Buff/窗口/表现驱动；默认不进存档）
 * - Persistent: 持久层（通常需要保存/跨会话；在 GAS 中通常对应 GE/GrantedTags）
 */
UENUM(BlueprintType)
enum class ESyStateWriteLayer : uint8
{
	Temporary UMETA(DisplayName = "Temporary"),
	Persistent UMETA(DisplayName = "Persistent"),
};

/**
 * FSyStateChangeRequest - 统一状态变更请求
 *
 * 设计目标：
 * - 成为 SyPlugins 体系里“唯一入口”的参数载体（通过 USyEntityStateFacadeComponent）
 * - 允许不同后端（Generic / GAS / 自研Numeric）按 Scope/Layer 路由和实现
 */
USTRUCT(BlueprintType)
struct SYCORE_API FSyStateChangeRequest
{
	GENERATED_BODY()

	/** 目标范围：Entity/Type/World */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyState")
	ESyStateScope Scope = ESyStateScope::Entity;

	/** 写入层级：Temporary/Persistent */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyState")
	ESyStateWriteLayer Layer = ESyStateWriteLayer::Temporary;

	/** 目标实体ID（Scope=Entity 时推荐必须有效；Facade 可做转发路由） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyState")
	FGuid TargetEntityId;

	/** 目标类型Tag（Scope=Type 时使用；兼容既有 StateManager 快照语义） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyState")
	FGameplayTag TargetTypeTag;

	/** 状态键 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyState")
	FGameplayTag StateTag;

	/** 状态值（由 TagMetadata/Schema 决定结构类型；GAS 后端可做映射） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyState")
	FInstancedStruct Value;

	/** 发起系统标识（用于审计/路由/调试；在持久写入时建议提供） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SyState")
	FGameplayTag SourceSystemTag;
};


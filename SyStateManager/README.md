# SyStateManager Module

## 概述

`SyStateManager` 模块是 `SyPlugins` 系统中的核心组件之一，其主要职责是作为**状态变更操作意图的中央日志记录器和事件广播中心**。

本模块的核心思想是**记录意图而非执行状态变更**。它接收由其他系统（如流程控制 `SyFlowImpl` 或游戏逻辑模块）创建的 `FSyOperation` 对象，将其封装为 `FSyStateModificationRecord` 进行持久化记录（在当前实现中是内存日志），并广播事件通知关心这些操作记录的系统（例如未来的 `SyEntity`/`USyStateComponent` 模块）。

`SyStateManager` 本身**不负责**解析 `FSyOperation` 的内容或直接修改任何游戏实体的状态。状态的实际应用由订阅其事件或查询其记录的下游系统负责。

## 核心概念

1.  **状态管理器子系统 (`USyStateManagerSubsystem`)**:
    *   实现为 `UWorldSubsystem`，每个游戏世界拥有一个实例。
    *   提供记录操作 (`RecordOperation`) 和查询记录 (`GetAggregatedModifications`, `GetAllModifications_Simple`) 的接口。
    *   维护一个内部的操作记录日志 (`ModificationLog`)。
    *   提供事件委托 (`OnStateModificationRecorded`)，在新的操作被记录时广播。

2.  **状态修改记录 (`FSyStateModificationRecord`)**:
    *   一个数据结构，用于封装一次被记录的操作意图。
    *   核心成员是 `FSyOperation Operation`，包含了原始操作的**完整信息**（Source, Modifier, Target）。
    *   包含额外的元数据，如记录自身的 `RecordId` 和创建时间戳 `Timestamp`。
    *   下游系统可以根据 `FSyStateModificationRecord` 中完整的 `FSyOperation` 信息进行灵活的筛选和处理。

## 主要功能

*   **记录操作 (`RecordOperation`)**: 接收 `FSyOperation`，进行基础验证，创建 `FSyStateModificationRecord`，存入日志，并广播 `OnStateModificationRecorded` 事件。
*   **聚合查询 (`GetAggregatedModifications`)**: 提供一个面向下游应用（如 `USyStateComponent`）的查询接口。根据传入的目标筛选条件 (`TargetFilterTag`，未来可扩展源筛选等)，遍历日志，将符合条件的记录中的 `FSyOperationModifier.StateModifications` 聚合到一个 `FSyStateParameterSet` 中返回。这简化了状态应用方的逻辑。
*   **简单查询 (`GetAllModifications_Simple`)**: 返回完整的内部日志记录，主要用于调试或需要完整历史记录的场景。
*   **事件通知 (`OnStateModificationRecorded`)**: 当 `RecordOperation` 成功时广播，允许其他系统实时响应新的操作记录。

## 工作流程

1.  **发起操作**: 某个系统 (如技能系统) 创建一个 `FSyOperation` 实例，描述想要进行的状态变更（例如：技能A对玩家B施加燃烧效果）。
2.  **记录意图**: 该系统获取当前世界的 `USyStateManagerSubsystem` 实例，并调用 `RecordOperation(MyOperation)`。
3.  **日志与广播**: `USyStateManagerSubsystem` 验证操作，创建包含 `MyOperation` 的 `FSyStateModificationRecord`，将其添加到 `ModificationLog` 中，并广播 `OnStateModificationRecorded` 事件，将新记录传递出去。
4.  **状态应用 (下游)**: 另一个系统（例如玩家B身上的 `USyStateComponent`）可能：
    *   订阅了 `OnStateModificationRecorded` 事件，实时收到新记录。
    *   或者，在自己的更新逻辑中定期调用 `GetAggregatedModifications(MyTargetTag)` 来查询与其相关的、聚合后的状态修改参数。
5.  **处理应用**: `USyStateComponent` 根据获取到的记录或聚合参数集，解析其中的状态标签和参数，并最终修改自身维护的状态数据（例如减少生命值、添加Buff状态对象等）。

## 依赖关系

*   **`Core`, `CoreUObject`, `Engine`**: Unreal Engine 核心模块。
*   **`GameplayTags`**: 用于 `FSyOperation` 中的标签定义和 `GetAggregatedModifications` 的筛选参数。
*   **`SyCore`**: 可能的基础类型或工具。
*   **`SyOperation`**: 定义了核心数据结构 `FSyOperation`。
*   **`SyStateCore`**: 定义了 `FSyStateParameterSet`，用于 `GetAggregatedModifications` 的返回值和 `FSyOperationModifier` 内部。

## 使用示例 (概念性)

```cpp
// --- 发起操作 (例如在某个技能 Actor 中) ---
#include "SyStateManagerSubsystem.h"
#include "OperationTypes.h"

void AMySkillActor::ApplyEffectToTarget(AActor* TargetActor)
{
    FSyOperation DamageOperation; // ... 填充 DamageOperation 的 Source, Modifier, Target ...

    // 获取 StateManager
    if (UWorld* World = GetWorld())
    {
        if (USyStateManagerSubsystem* StateManager = World->GetSubsystem<USyStateManagerSubsystem>())
        {
            // 记录操作意图
            StateManager->RecordOperation(DamageOperation);
        }
    }
}

// --- 应用状态 (例如在 USyStateComponent 中) ---
#include "SyStateManagerSubsystem.h"
#include "GameplayTagContainer.h"
#include "StateParameterTypes.h"

void USyStateComponent::UpdateState()
{
    if (UWorld* World = GetWorld())
    {
        if (USyStateManagerSubsystem* StateManager = World->GetSubsystem<USyStateManagerSubsystem>())
        {
            // 假设本组件代表的目标类型是 "EntityType.Character.Player"
            FGameplayTag MyTargetTag = FGameplayTag::RequestGameplayTag("EntityType.Character.Player");

            // 获取聚合后的修改参数
            FSyStateParameterSet ModificationsToApply = StateManager->GetAggregatedModifications(MyTargetTag);

            // 处理聚合后的参数集
            ApplyAggregatedModifications(ModificationsToApply);
        }
    }
}

void USyStateComponent::ApplyAggregatedModifications(const FSyStateParameterSet& Modifications)
{
    // 遍历 Modifications.GetStateParamsMap()
    // 根据 StateTag 和 Params 处理状态变更...
}

// 或者通过订阅事件实时处理
void USyStateComponent::OnStateRecordReceived(const FSyStateModificationRecord& NewRecord)
{
    // 检查 NewRecord.Operation.Target.TargetTypeTag 是否与自身匹配
    // 如果匹配，则处理 NewRecord.Operation.Modifier.StateModifications...
}
```

## 未来拓展方向 (代码中 TODO 标记)

*   **查询优化**: 为常用查询（如按目标类型、源类型）添加内部索引，提高 `GetAggregatedModifications` 的性能。
*   **网络同步**: 实现 `FSyStateModificationRecord` 的网络复制或将 `RecordOperation` 事件同步到客户端。
*   **日志管理**: 实现日志大小限制和清理策略（例如 FIFO、基于时间）。
*   **持久化**: 将 `ModificationLog` 保存到 SaveGame 或数据库。
*   **更复杂的筛选**: 在 `GetAggregatedModifications` 中支持基于源信息 (`Source`) 和目标参数 (`Target.Parameters`) 的筛选。
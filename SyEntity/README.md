# SyEntity - 通用实体框架模块

## 1. 模块概述 (Module Overview)

`SyEntity` 提供了一个基于组件的通用游戏实体管理框架。它允许开发者以非侵入式的方式为任何 Actor 添加实体标识、状态管理、消息处理和组件协调等核心能力。

本模块的核心是 `USyEntityComponent`，它作为实体能力的入口点，并自动创建和管理其他关键的功能组件，如 `USyIdentityComponent` (来自 `SyCore`) 和 `USyStateComponent`。

## 2. 核心设计理念 (Core Design Philosophy)

*   **组件化管理:** `USyEntityComponent` 作为核心协调器，自动管理必要的 Sy* 功能组件。Actor 只需添加 `USyEntityComponent` 即可获得基础实体能力。
*   **非侵入式:** 无需修改现有 Actor 继承结构。
*   **统一状态模型 (通过 `SyStateComponent` & `SyStateCore`):**
    *   **运行时状态容器:** `USyStateComponent` 内部持有并管理 `FSyStateCategories` (来自 `SyStateCore`)，用于存储实体当前的、结构化的运行时状态。状态数据由具体的 `USyStateMetadataBase` 子类对象实例承载。
    *   **数据驱动初始化:** 实体的初始状态通过 `USyStateComponent` 的 `FSyStateParameterSet DefaultInitData` 进行配置，`BeginPlay` 时自动应用。
    *   **全局同步与本地状态 (可选):**
        *   `USyStateComponent` 可通过蓝图配置的 `bEnableGlobalSync` 标志决定是否与全局 `USyStateManagerSubsystem` 同步。
        *   启用同步时，组件会在 `BeginPlay` 时连接到 `StateManager`，订阅状态修改记录事件，并拉取历史聚合状态进行初始化同步。
        *   运行时，监听 `StateManager` 广播的记录，如果记录与自身的 `TargetTypeTag` 相关，则重新从 `StateManager` 获取**最新的聚合状态参数集** (`FSyStateParameterSet`) 并将其应用到本地的 `FSyStateCategories` 中，实现**基于最终结果的覆盖式同步**。
        *   禁用同步时，组件将只管理本地状态，不与 `StateManager` 交互。
*   **事件驱动更新通知:** `USyStateComponent` 在其本地状态数据实际发生变更时（初始化或应用聚合修改后）会广播 `OnLocalStateDataChanged` 事件。`USyEntityComponent` 监听此事件并广播自己的 `OnEntityStateUpdated` 事件，通知其他关心实体状态变化的系统。

## 3. 模块架构 (Module Architecture)

### 3.1 核心组件

*   **`USyEntityComponent`**:
    *   **角色:** 框架核心，实体能力的入口和协调器。
    *   **职责:**
        *   自动创建并管理核心依赖组件 (`USyIdentityComponent`, `USyStateComponent`, `USyMessageComponent`)。
        *   提供访问这些核心组件的接口。
        *   监听 `USyStateComponent` 的 `OnLocalStateDataChanged` 事件，并广播实体级的 `OnEntityStateUpdated` 事件。
        *   提供实体 ID、Tags、Alias 的访问接口 (代理 `USyIdentityComponent`)。
        *   提供消息发送接口 (代理 `USyMessageComponent`)。
*   **`USyStateComponent`**:
    *   **角色:** 实体状态管理的核心实现。
    *   **职责:**
        *   持有并管理运行时的 `FSyStateCategories`。
        *   通过 `DefaultInitData` (`FSyStateParameterSet`) 进行初始化。
        *   (可选) 通过 `bEnableGlobalSync` 控制是否连接 `USyStateManagerSubsystem`。
        *   (可选) 订阅 `StateManager` 的 `OnStateModificationRecorded` 事件。
        *   (可选) 在收到相关记录时，调用 `StateManager->GetAggregatedModifications()` 获取最新的聚合状态。
        *   调用 `FSyStateCategories::ApplyStateModifications()` 应用聚合后的状态参数。
        *   提供 `GetCurrentStateCategories()` 接口用于外部查询状态。
        *   提供 `OnLocalStateDataChanged` 事件通知本地数据变更。

### 3.2 其他关联组件/系统

*   **`USyIdentityComponent` (来自 `SyCore`):** 提供实体唯一标识。
*   **`USyMessageComponent` (来自 `SyCore`):** 提供消息发送/接收能力。
*   **交互组件 (可选):** 处理具体的 Gameplay 交互逻辑。
*   **`USyStateManagerSubsystem` (来自 `SyStateManager`):** 全局状态记录和广播中心。
*   **`FSyStateCategories`, `FSyStateParameterSet`, `USyStateMetadataBase` (来自 `SyStateCore`):** 核心状态数据结构。
*   **`FSyOperation` (来自 `SyOperation`):** 状态操作意图的数据结构，由 `StateManager` 记录。
*   **`SyEntityRegistry` (可选):** 维护活动实体索引。

## 4. 工作流程 (Workflows)

### 4.1 实体创建与初始化

1.  为 Actor 添加 `USyEntityComponent`。
2.  `USyEntityComponent` 在 `BeginPlay` 时自动确保 `USyIdentityComponent`, `USyStateComponent`, `USyMessageComponent` 被创建和注册。
3.  `USyEntityComponent` 调用 `InitializeEntity` (内部可能已在 BeginPlay 自动调用)，绑定内部委托。
4.  `USyStateComponent` 在其 `BeginPlay` 中：
    *   调用 `ApplyInitializationData(DefaultInitData)` 应用配置的初始状态。
    *   如果 `bEnableGlobalSync` 为 true:
        *   调用 `TryConnectToStateManager()` 获取 `StateManagerSubsystem` 实例并订阅 `OnStateModificationRecorded` 事件。
        *   调用 `ApplyAggregatedModifications()` 从 `StateManager` 拉取历史聚合状态并应用，完成初始同步。
    *   `ApplyInitializationData` 和 `ApplyAggregatedModifications` 内部会在状态实际改变后广播 `OnLocalStateDataChanged`。
5.  `USyEntityComponent` 监听到 `OnLocalStateDataChanged` 后广播 `OnEntityStateUpdated`。
6.  实体通过 `SyEntityRegistry` 注册自身（如果使用）。

### 4.2 运行时状态同步 (当 `bEnableGlobalSync` 为 true)

1.  外部系统创建 `FSyOperation` 并通过 `StateManager->RecordOperation()` 记录。
2.  `StateManager` 广播 `OnStateModificationRecorded` 事件。
3.  所有已连接的 `USyStateComponent` 实例在其 `HandleStateModificationRecorded` 回调中被唤醒。
4.  每个 `USyStateComponent` 检查收到的 `Record` 中的 `Operation.Target.TargetTypeTag` 是否与自身的 `TargetTypeTag` 匹配。
5.  如果匹配，该 `USyStateComponent` 调用 `ApplyAggregatedModifications()`。
6.  `ApplyAggregatedModifications` 内部调用 `StateManager->GetAggregatedModifications(TargetTypeTag)` 获取**当前最新的聚合状态参数集** (`FSyStateParameterSet`)。
7.  调用 `CurrentStateCategories.ApplyStateModifications(AggregatedMods.Parameters)` 将最新的聚合状态**覆盖或合并**到本地状态容器中。
8.  `ApplyStateModifications` 内部改变状态后，广播 `OnLocalStateDataChanged`。
9.  `USyEntityComponent` 监听到 `OnLocalStateDataChanged` 后广播 `OnEntityStateUpdated`。

### 4.3 状态访问

*   其他组件或蓝图需要读取实体状态时，应通过 `USyEntityComponent::GetStateComponent()` 获取 `USyStateComponent` 引用，然后调用 `GetCurrentStateCategories()` 获取状态容器，最后使用 `FSyStateCategories` 提供的接口（如 `FindFirstStateMetadata<T>`）来查询具体状态数据。

## 5. 依赖关系 (Dependencies)

*   `SyCore`
*   `SyStateCore`
*   `SyStateManager`
*   `SyOperation`
*   `CoreUObject`
*   `Engine`
*   `GameplayTags`
*   `TagMetadata` (间接通过 `SyStateCore`)
*   `StructUtils` (间接通过 `SyStateCore` / `SyOperation`)

## 6. 使用建议与最佳实践 (Usage Notes/Guidelines)

*   将 `USyEntityComponent` 作为管理实体核心能力的入口。
*   在 Actor 蓝图或 C++ 构造函数中配置 `USyStateComponent` 的 `DefaultInitData` 和 `TargetTypeTag`。
*   根据需要设置 `USyStateComponent` 的 `bEnableGlobalSync`。
*   需要响应状态变化的逻辑，应监听 `USyEntityComponent` 的 `OnEntityStateUpdated` 事件。
*   需要读取状态值的逻辑，应通过 `USyEntityComponent -> USyStateComponent -> FSyStateCategories` 的路径访问。
*   确保 `FSyStateCategories::ApplyStateModifications` 方法实现了正确的状态覆盖或合并逻辑，这是同步准确性的关键。

## 7. 未来考虑 (Future Considerations)

*   为 `USyStateComponent::OnLocalStateDataChanged` 添加参数，传递更具体的状态变更信息。
*   优化 `StateManager` 的订阅机制，减少不必要的 `HandleStateModificationRecorded` 调用。
*   添加 `EntityId` 支持，实现更精确的目标匹配。
*   网络复制策略。
*   完善组件间通信方案。

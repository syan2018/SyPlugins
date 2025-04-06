# SyEntity - 通用实体框架模块

## 1. 模块概述 (Module Overview)

SyEntity 提供了一个基于组件的通用游戏实体管理框架。它允许开发者以非侵入式的方式为任何 Actor 添加实体标识、状态管理、组件协调等核心能力。本模块的核心是 `SyEntityComponent`，它作为实体能力的入口点，并协调其他功能组件（尤其是 `SyStateComponent`）。

## 2. 核心设计理念 (Core Design Philosophy)

*   **组件化管理:** `SyEntityComponent` 作为核心协调器，管理其他附加的功能组件（状态、交互、消息等）。Actor 只需添加 `SyEntityComponent` 即可接入框架。
*   **非侵入式:** 无需修改现有 Actor 继承结构。
*   **双向状态管理 (通过 `SyStateComponent`):**
    *   **本地状态维护:** `SyStateComponent` 负责维护实体当前的运行时状态 (`FSyEntityState`)，状态数据由具体的 `UO_TagMetadata` 对象实例承载。
    *   **全局同步 (可选):** `SyStateComponent` 可以连接到全局 `SyStateManager`，通过监听 `FSyStateModificationRecord` 来更新本地状态，实现与全局状态的同步。实体也可以选择完全独立管理状态。
*   **数据驱动:** 实体的初始状态、类型等应可通过数据资产或配置进行定义。

## 3. 模块架构 (Module Architecture)

### 3.1 核心组件

*   **`SyEntityComponent`**:
    *   **角色:** 框架核心，实体能力的入口和协调器。
    *   **职责:**
        *   管理和索引附属的功能组件（如 State, Messaging, Identity）。
        *   触发附属组件的初始化流程（如 Identity, State）。
        *   提供实体级别的接口和事件。
        *   （可选）协调组件间的通信。
*   **`USyStateComponent`**:
    *   **角色:** 实体状态管理的核心实现。
    *   **职责:**
        *   持有并管理运行时的 `FSyEntityState` (包含 `TMap<Tag, TArray<TObjectPtr<UO_TagMetadata>>>`)。
        *   根据 `FSyEntityInitData` (来自配置或 `SyEntityComponent`) 初始化状态，创建初始的 `UO_TagMetadata` 对象实例。
        *   连接到 `USyStateManager` (如果配置如此)，订阅或查询 `FSyStateModificationRecord`。
        *   解析 `FSyStateModificationRecord` 并应用状态变更到本地的 `UO_TagMetadata` 对象实例 (`ApplyStateModification` 逻辑)。
        *   提供查询本地状态数据的接口 (e.g., `FindFirstStateMetadata<T>`)。
        *   提供状态变更的本地通知机制（可选）。

### 3.2 其他关联组件/系统

*   **`SyIdentityComponent` (来自 SyCore):** 提供实体唯一标识。通常由 `SyEntityComponent` 触发其初始化。
*   **`SyMessagingComponent` (来自 SyCore):** 提供消息发送/接收能力。
*   **交互组件 (可选，来自 SyGameplay 或其他):** 处理具体的 Gameplay 交互逻辑。
*   **`SyStateManager` (来自 SyStateManager):** 全局状态记录和广播中心。`USyStateComponent` 与之交互以实现状态同步。
*   **`SyEntityRegistry`** 维护活动实体索引，支持查询。

## 4. 工作流程 (Workflows)

### 4.1 实体创建与初始化

1.  为 Actor 添加 `SyEntityComponent`。
2.  `SyEntityComponent` 在初始化时（如 `BeginPlay`）确保必要的依赖组件（如 `SyIdentityComponent`, `SyStateComponent`, `SyMessagingComponent`）存在并触发它们的初始化。
3.  `SyStateComponent` 调用 `InitializeState`，读取 `FSyEntityInitData` (可能由 `SyEntityComponent` 提供或自身配置)，创建初始的 `UO_TagMetadata` 状态对象实例存入 `FSyEntityState`。
4.  实体通过 `SyEntityRegistry` 注册自身（如果使用注册表）。
5.  `SyStateComponent` 根据配置连接到 `USyStateManager` 并订阅事件。

### 4.2 状态管理流程

1.  **配置:** 通过 `FSyEntityInitData` 定义实体类型的初始状态（使用初始化参数 `USTRUCT` 实例存储在 `FInstancedStruct` 中）。通过 `UO_TagMetadata` 子类定义具体的状态数据结构。
2.  **初始化:** `USyStateComponent` 在 `InitializeState` 时，根据 `InitData` 创建对应的 `UO_TagMetadata` 实例。
3.  **更新:**
    *   **本地修改 (如果允许):** 实体逻辑直接修改 `USyStateComponent` 持有的 `UO_TagMetadata` 对象属性。
    *   **全局同步:** `USyStateComponent` 接收到 `StateManager` 的 `FSyStateModificationRecord`，调用 `ApplyStateModification` 解析记录中的参数 (`FInstancedStruct ParametersApplied`)，查找并修改对应的本地 `UO_TagMetadata` 对象。
4.  **查询:** 实体逻辑通过 `USyStateComponent` 提供的接口查询当前状态数据。

### 4.3 组件间通信

*   **建议方式:** 优先使用 `SyCore` 的 `MessageBus` 进行解耦通信。例如，`USyStateComponent` 在状态变更后可以发送一个消息，其他组件（如交互组件）可以监听这个消息来启用/禁用自身。
*   **直接调用 (谨慎使用):** `SyEntityComponent` 可以提供接口来获取其他关联组件的引用，但应尽量避免紧耦合。

## 5. 依赖关系 (Dependencies)

*   `SyCore`
*   `SyStateCore`
*   `SyStateManager`
*   `SyOperation`
*   `CoreUObject`
*   `Engine`
*   `GameplayTags`
*   `TagMetadata`
*   `StructUtils`

## 6. 使用建议与最佳实践 (Usage Notes/Guidelines)

*   将 `SyEntityComponent` 作为管理实体核心能力的入口。
*   仔细设计 `FSyEntityInitData` 和对应的 `UO_TagMetadata` 状态类。
*   `ApplyStateModification` 是 `USyStateComponent` 中实现状态效果的核心逻辑点。
*   正确管理 `UO_TagMetadata` 状态对象的生命周期（尤其是在 Buff 或临时状态移除时）。
*   考虑清楚哪些状态需要全局同步，哪些可以保持纯本地。

## 7. 未来考虑 (Future Considerations)

*   添加状态变更的本地事件委托。
*   优化 `UO_TagMetadata` 对象的网络复制策略。
*   提供更丰富的状态查询接口。
*   完善组件间通信的最佳实践方案。
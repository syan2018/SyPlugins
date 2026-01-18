# SyCore - 核心基础模块

## 模块概述 (Core Purpose)

SyCore 是 SyPlugins 框架的最基础模块，定义了全局共享的 Gameplay Tag 结构，并提供了实体唯一标识 (`Identifier`) 和模块间通信 (`MessageBus`) 等核心基础设施服务。该模块作为所有其他 SyPlugins 高层模块的基础，确保系统的可靠性、一致性和可扩展性。

## 关键职责 (Key Responsibilities)

*   **定义基础 Gameplay Tag 结构:** 在 `Config/DefaultGameplayTags.ini` 或相关数据资产中定义和维护基础标签树 (e.g., `Sy.`, `Sy.State.`, `Sy.Operation.`)。
*   **提供标识系统 (`Identifier`):** 通过 `SyIdentityComponent` (或其他机制) 生成和管理实体唯一标识符 (UUID) 和可选别名，支持实体查找和引用。
*   **提供消息总线 (`MessageBus`):** 实现模块间和实体间的异步消息传递，支持消息订阅、发布和优先级。
*   **(可选) 定义框架级接口:** 定义 SyPlugins 模块可能需要遵循的基础接口或基类。
*   **(可选) 基础功能集成:** 封装或暴露基础引擎/插件功能（如 `TagMetadata` 的基础访问）供其他模块统一使用。

## 核心组件与概念 (Core Components & Concepts)

### 1. 基础服务 (Foundation & Infrastructure)
*   **Gameplay Tags:** 系统范围内的核心标识符，定义于配置文件或数据资产中。
*   **MessageBus (消息总线):**
    *   实现：包含消息组件（如 `SyMessageComponent`）和中央总线逻辑（`USyMessageBus`）。
    *   消息结构 (`FSyMessage`)：
        *   **来源 (`FSyMessageSource`):** 来源身份GameplayTag + 来源标识 (`FGuid`, 别名)。
        *   **内容 (`FSyMessageContent`):** 消息类型GameplayTag + `FInstancedStruct` Payload（支持任意结构化数据）+ 可选元数据键值对。
        *   **元信息:** 时间戳、优先级 (`ESyMessagePriority`)、消息唯一ID。
    *   用途：主要用于事件驱动的逻辑解耦，如 Flow 监听特定消息。

### 2. 实体系统 (Entity System)
*   **SyEntityComponent:** 实体的核心组件，作为其他所有Sy组件的挂载点和协调者。
*   **Identity (标识系统):**
    *   实现：`SyIdentityComponent`。
    *   功能：管理实体的唯一 `FGuid` 和可选的 `FName` 别名。
    *   职责：提供实体的身份识别，支持通过ID或别名查找实体。

### 3. 状态系统 (State System)
*(原 SyStateSystem, SyOperation 模块已合并至此)*
*   **SyStateComponent:** 管理实体的状态数据。
*   **StateManager:** 全局状态管理子系统，负责状态变更的记录、聚合和分发。
*   **SyOperation:** 定义对状态的操作（来源、目标、修改器）。
*   **状态数据结构:**
    *   `FSyStateCategories`: 状态集合。
    *   `FSyStateParams`: 具体的状态参数。

## 目录结构

```tree
SyCore/
├── Source/
│   ├── SyCore/
│   │   ├── Private/
│   │   │   ├── Entity/      (实体系统: SyEntityComponent, Identity)
│   │   │   ├── Foundation/  (基础工具)
│   │   │   ├── Messaging/   (消息与事件系统)
│   │   │   ├── State/       (状态系统: StateManager, Operations, Components)
│   │   │   └── SyCore.cpp
│   │   └── Public/
│   │       ├── SyCore.h
│   │       ├── Entity/
│   │       ├── Foundation/
│   │       ├── Messaging/
│   │       └── State/
│   ├── SyCoreEditor/        (Editor模块)
│   │   ├── Private/
│   │   └── Public/
│   └── SyCore.Build.cs
├── SyCore.uplugin
└── README.md
```

## 使用指南 (Usage Guide)

### Entity 实体模块
*   **初始化:** 为 Actor 添加 `USyEntityComponent`。该组件会自动处理依赖组件（如 Identity, State, Message）的创建和初始化顺序。
*   **使用:** 通过 `SyEntityComponent` 访问其他功能组件。

### State 状态模块
*   **操作记录:** 使用 `USyStateManagerSubsystem` 记录 `FSyOperation` 以修改实体状态。
*   **状态监听:** `USyStateComponent` 会自动响应相关的状态变更。

### MessageBus 消息总线
*   **发送:** 通过 `SyMessageComponent` 发送结构化消息。
*   **监听:** 订阅特定Tag的消息以进行解耦通信。

## 依赖关系 (Dependencies)

*   `Core`
*   `CoreUObject`
*   `GameplayTags`
*   `StructUtils` (用于 `FInstancedStruct` 支持)
*   `TagMetadata` (用于基础功能暴露，可选)

## 使用建议与最佳实践 (Usage Notes/Guidelines)

*   所有其他 SyPlugins 模块都应依赖于 `SyCore`。
*   保持 `SyCore` 的基础性，避免在此处添加高层业务逻辑。
*   Gameplay Tag 的管理应集中进行，保持结构清晰。
*   优先使用消息通信实现模块解耦。
*   正确管理组件（Identity, Messaging）的生命周期和初始化顺序。

## 未来考虑 (Future Considerations)

*   可能会添加全局配置 Subsystem 来管理 SyPlugins 范围内的通用设置。
*   完善 `TagMetadata` 在消息内容结构化方面的应用。

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

*   **Gameplay Tags:** 系统范围内的核心标识符，定义于配置文件或数据资产中。
*   **Identifier (标识系统):**
    *   实现：通常通过 `SyIdentityComponent`。
    *   功能：管理实体的唯一 `FGuid` 和可选的 `FName` 别名。
*   **MessageBus (消息总线):**
    *   实现：包含消息组件（如 `SyMessageComponent`）和中央总线逻辑（`USyMessageBus`）。
    *   消息结构 (`FSyMessage`)：
        *   **来源 (`FSyMessageSource`):** 来源身份GameplayTag + 来源标识 (`FGuid`, 别名)。
        *   **内容 (`FSyMessageContent`):** 消息类型GameplayTag + `FInstancedStruct` Payload（支持任意结构化数据）+ 可选元数据键值对。
        *   **元信息:** 时间戳、优先级 (`ESyMessagePriority`)、消息唯一ID。
    *   用途：主要用于事件驱动的逻辑解耦，如 Flow 监听特定消息。
*   **(Future) `ISyModuleInterface`:** (如果需要) 定义 SyPlugins 模块通用的接口规范。

## 目录结构

```tree
SyCore/
├── Source/
│   ├── SyCore/
│   │   ├── Private/
│   │   │   ├── Foundation/  (基础工具)
│   │   │   ├── Identity/    (ID和引用管理 - SyIdentityComponent 实现)
│   │   │   ├── Messaging/   (消息与事件系统 - SyMessagingComponent, MessageBus 实现)
│   │   │   └── SyCore.cpp
│   │   └── Public/
│   │       ├── SyCore.h
│   │       ├── Foundation/
│   │       │   └── Utilities/
│   │       ├── Identity/
│   │       │   └── (Public Headers: FSyIdentifier, Interfaces)
│   │       └── Messaging/
│   │           └── (Public Headers: FSyMessage, Interfaces)
│   └── SyCore.Build.cs
├── SyCore.uplugin
└── README.md
```

## 使用指南 (Usage Guide)

### Identity 身份标识模块

*   **初始化:** 由于 ActorComponent 限制，`SyIdentityComponent` 通常需要在 Actor 的 Construction Script 或 `BeginPlay` 中显式调用初始化逻辑（例如，通过宿主 Actor 或 `SyEntityComponent` 触发），以生成或分配 UUID。
*   **依赖:** 消息系统等依赖于 Identity 组件的有效初始化。
*   **配置:** 可在 Actor 模板或实例上配置代表实体类型的 Gameplay Tag。

### MessageBus 消息总线

*   **初始化:** 依赖于 `SyIdentityComponent` 和消息组件的初始化。
*   **发送:** 通过消息组件接口发送结构化的 `FSyMessage`。
*   **监听:** 其他系统（如 `SyFlow`, `SyQuest`）通过 MessageBus 订阅感兴趣的来源 Tag 或消息 Tag。

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
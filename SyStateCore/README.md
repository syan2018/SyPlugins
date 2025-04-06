# SyStateCore Module

## Core Purpose

定义与实体状态表示和初始化相关的数据结构，以及承载具体状态数据的 `UO_TagMetadata` 基类和子类。

## Key Responsibilities

*   定义实体当前运行时状态的数据容器 (`FSyEntityState`)。
*   定义实体初始化时使用的状态配置结构 (`FSyEntityInitData`)。
*   提供各种用于承载具体状态数据的 `UO_TagMetadata` 子类（如 `USyHealthMetadata`, `USyStatusEffectMetadata` 等）的定义。
*   定义用于 `FSyEntityInitData` 的初始化参数 `USTRUCT`（如 `FHealthInitParams`）。
*   提供访问 `FSyEntityState` 中状态数据的辅助函数或接口（可选）。

## Core Concepts & Types

*   **`FSyEntityState`:** 包含 `TMap<FGameplayTag, TArray<TObjectPtr<UO_TagMetadata>>>`，用于存储实体运行时状态对象实例。
*   **`FSyEntityInitData`:** 包含 `TMap<FGameplayTag, TArray<FInstancedStruct>>`，用于在编辑器或蓝图中配置实体初始状态的参数。
*   **`UO_TagMetadata` (State Subclasses):** 如 `USyHealthMetadata`, `USyManaMetadata`, `USyStatusEffectMetadata` 等，定义具体状态数据的结构和属性。这些类的实例将存储在 `FSyEntityState` 中。
*   **Initialization Parameter Structs:** 如 `FHealthInitParams`，与 `UO_TagMetadata` 状态子类对应，用于在 `FSyEntityInitData` 中传递初始化值。

## Dependencies

*   `SyCore`
*   `CoreUObject`
*   `GameplayTags`
*   `TagMetadata` (用于 `UO_TagMetadata`)
*   `StructUtils` (用于 `FInstancedStruct`)

## Usage Notes/Guidelines

*   当需要为实体添加新的状态类型时（如“怒气值”），应在此模块中创建新的 `UO_TagMetadata` 子类（如 `USyRageMetadata`）和可选的初始化参数 `USTRUCT`。
*   `FSyEntityState` 中的 `UO_TagMetadata` 对象实例由 `USyStateComponent` (SyEntity 模块) 负责创建和管理。
*   `FSyEntityInitData` 主要用于配置和初始化，运行时状态由 `FSyEntityState` 表示。

## Future Considerations

*   可能添加更通用的状态元数据基类或接口。
*   优化状态数据的序列化和网络复制（如果需要）。
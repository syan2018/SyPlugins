# SyStateCore Module

## Core Purpose

定义与实体状态表示和初始化相关的数据结构，以及承载具体状态数据的 `USyStateMetadataBase` 基类和子类。本模块提供了状态管理系统的核心数据结构和类型，支持通过 GameplayTag 系统进行状态分类和管理。

## Key Responsibilities

*   定义实体当前运行时状态的数据容器 (`FSyStateCategories`)。
*   定义用于配置实体初始状态或修改状态的参数结构 (`FSyStateParameterSet`, `FSyStateParams`)。
*   提供各种用于承载具体状态数据的 `USyStateMetadataBase` 子类（如 `USyTagStateMetadata` 等）的定义。
*   提供访问 `FSyStateCategories` 中状态数据的辅助函数或接口。

## Core Concepts & Types

*   **`FSyStateCategories`:** 包含 `TMap<FGameplayTag, FSyStateMetadatas>`，用于存储实体运行时状态对象实例。提供了查找、添加、移除状态元数据以及批量应用初始化和修改的方法。
*   **`FSyStateMetadatas`:** 包含 `TArray<TObjectPtr<UO_TagMetadata>>`，用于存储特定状态标签的多个元数据对象实例。
*   **`FSyStateParameterSet`:** 包含 `TMap<FGameplayTag, FSyStateParams>`，用于在编辑器或蓝图中配置实体初始状态或运行时修改的参数。
*   **`FSyStateParams`:** 包含 `TArray<FSyInstancedStruct>`，用于存储对特定状态标签的多个参数（使用 `SyCore` 中的 `FSyInstancedStruct`）。
*   **`USyStateMetadataBase`:** 所有状态元数据的基类，继承自 `UO_TagMetadata`，提供状态标签、参数处理（通过 `FInstancedStruct`）和值访问的基本功能。每个子类应实现 `GetValueDataType` 以标识其管理的具体数据类型（`USTRUCT`）。
*   **`USyTagStateMetadata`:** 用于存储 `FGameplayTag` 类型状态值的 `USyStateMetadataBase` 具体实现。

## Dependencies

*   `SyCore` (用于 `FSyInstancedStruct`)
*   `CoreUObject`
*   `GameplayTags`
*   `TagMetadata` (用于 `UO_TagMetadata`)
*   `StructUtils` (用于 `FInstancedStruct`)

## Usage Notes/Guidelines

*   当需要为实体添加新的状态类型时（如"怒气值"），应在此模块中创建新的 `USyStateMetadataBase` 子类（如 `USyRageMetadata`）和用于存储其值的 `USTRUCT`。
*   `FSyStateCategories` 中的 `UO_TagMetadata` 对象实例由外部系统（如 `USyStateComponent`）负责创建和管理。
*   `FSyStateParameterSet` 主要用于配置和初始化，或传递状态修改参数；运行时状态由 `FSyStateCategories` 表示。
*   使用 `GetValueStruct` 和 `SetValueStruct` 方法（操作 `FInstancedStruct`）在蓝图中或通用代码中访问和修改状态值。
*   使用 `GetValue<T>()` 和 `SetValue<T>()` 模板方法（操作具体 `USTRUCT` 类型）在 C++ 中进行类型安全的状态值访问和修改。
*   子类需要实现 `GetValueDataType_Implementation` 返回其关联的 `USTRUCT` 类型。

蓝图拓展可参考 `Content\SM_StateMetadataSample.uasset`

## Implementation Details

### 状态元数据基类 (USyStateMetadataBase)

`USyStateMetadataBase` 是所有状态元数据的基类，提供了以下关键功能：

*   **类型识别**：通过 `GetValueDataType` (BlueprintNativeEvent) 返回其管理的具体 `USTRUCT` 类型。
*   **状态标签管理**：通过 `GetStateTag` 和 `SetStateTag` 方法管理状态标签。
*   **值访问接口 (通用)**：通过 `GetValueStruct` 和 `SetValueStruct` (BlueprintNativeEvent) 方法提供通用的基于 `FInstancedStruct` 的值访问接口。
*   **值访问接口 (C++)**：通过 `GetValue<T>()` 和 `SetValue<T>()` 模板方法提供类型安全的基于具体 `USTRUCT` 类型的值访问。
*   **参数处理**：通过 `InitializeFromParams` 和 `ApplyModification` 方法处理初始化和修改参数（接收 `FInstancedStruct`）。

### 状态分类 (FSyStateCategories)

`FSyStateCategories` 是存储实体运行时状态的核心数据结构，提供了以下功能：

*   **状态查找**：通过 `FindFirstStateMetadata<T>` 和 `GetAllStateMetadata<T>` 模板方法查找特定类型的状态元数据。
*   **状态管理**：通过 `AddStateMetadata`、`RemoveStateMetadata` 和 `ClearStateMetadata` 方法管理状态元数据对象。
*   **批量应用**：通过 `ApplyInitData` (接收 `FSyStateParameterSet`) 和 `ApplyStateModifications` (接收 `TMap<FGameplayTag, FSyStateParams>`) 方法批量应用初始化和修改。

## Future Considerations

*   可能添加更通用的状态元数据基类或接口。
*   优化状态数据的序列化和网络复制（如果需要）。
*   添加更多类型的状态元数据子类，如 `USyFloatStateMetadata`、`USyIntStateMetadata` 等，以及它们对应的 `USTRUCT` 定义。
*   实现状态数据的编辑器工具，简化状态配置和调试（例如，利用 `GetValueDataType` 实现参数类型的自动推断）。
*   添加状态数据的验证和约束机制，确保状态数据的一致性。
# SyOperation Module

## Core Purpose

定义状态变更操作 (`FSyOperation`) 的数据结构、操作参数的 Schema 定义机制 (`USyOperationParamSchemaMetadata`) 以及具体的参数结构体 (`USTRUCT`)。

## Key Responsibilities

*   定义 `FSyOperation` 结构体及其子结构 (`FSyOperationSource`, `FSyOperationModifier`, `FSyOperationTarget`)，使用 `GameplayTag` 和 `FInstancedStruct`。
*   定义 `USyOperationParamSchemaMetadata` 类，用于通过 `TagMetadata` 插件将 `GameplayTag` 与期望的参数 `UScriptStruct*` 类型关联起来。
*   提供各种操作（如伤害、状态应用、交互）所需的具体参数 `USTRUCT` 定义（如 `FDamageParams`, `FApplyStatusParams`）。

## Core Concepts & Types

*   **`FSyOperation`**: 代表一个完整的状态变更意图，包含来源、修饰器和目标。
*   **`FSyOperationSource`, `FSyOperationModifier`, `FSyOperationTarget`**: 操作的组成部分，包含 `GameplayTag` 和 `FInstancedStruct Parameters`。
*   **`USyOperationParamSchemaMetadata`**: 继承自 `UO_TagMetadata`，用于在 `TagMetadataCollection` 中定义操作 Tag 对应的参数结构体 Schema。
*   **Parameter Structs (`USTRUCT`)**: 如 `FDamageParams`, `FApplyStatusParams`, `FInteractionSourceParams` 等，定义具体操作的参数字段。这些结构体的实例将存储在 `FInstancedStruct Parameters` 中。

## Dependencies

*   `SyCore`
*   `CoreUObject`
*   `GameplayTags`
*   `TagMetadata`
*   `StructUtils`

## Usage Notes/Guidelines

*   当需要定义新的操作类型或参数时，应在此模块中创建新的参数 `USTRUCT`。
*   需要在 `UO_TagMetadataCollection` 数据资产中，将操作相关的 `GameplayTag` 与 `USyOperationParamSchemaMetadata` 关联，并指定正确的参数 `USTRUCT` 类型。
*   系统发起操作时，应先查询 Tag 关联的 Schema 获取参数类型，然后创建并填充 `FInstancedStruct Parameters`。
*   考虑实现编辑器自定义（`PostEditChangeProperty` 或 `IDetailCustomization`）来改善编辑器中 Tag 与 `FInstancedStruct` 参数类型的联动体验。

## Future Considerations

*   添加更多标准化的操作参数结构体。
*   提供辅助函数或蓝图库来简化 Operation 的创建和参数填充。
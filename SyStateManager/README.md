# SyStateManager Module

## Core Purpose

提供中心化的状态变更记录和分发机制，作为状态同步的权威来源。

## Key Responsibilities

*   定义状态变更记录的数据结构 (`FSyStateModificationRecord`)。
*   提供 `USyStateManager` 子系统，负责接收 `FSyOperation`。
*   验证传入的 `FSyOperation`（可选）。
*   根据 `FSyOperation` 创建并存储 `FSyStateModificationRecord`。
*   提供查询接口供 `USyStateComponent` 获取状态变更记录。
*   提供事件委托 (`OnStateModificationRecorded`)，在记录创建时广播通知。
*   **不直接修改**任何 `USyStateComponent` 的状态。

## Core Concepts & Types

*   **`USyStateManager`**: 通常实现为 `UGameInstanceSubsystem` 或 `UWorldSubsystem`，是状态记录和分发的中心。
*   **`FSyStateModificationRecord`**: 存储一次已处理的操作修改信息，包括目标实体 ID、修改 Tag、应用的参数 (`FInstancedStruct ParametersApplied`) 和时间戳等。
*   **`OnStateModificationRecorded`**: `USyStateManager` 上的委托，当新的修改记录产生时触发。

## Dependencies

*   `SyCore`
*   `SyOperation` (需要 `FSyOperation` 和参数结构体定义来创建 Record)
*   `SyStateCore` (可能需要实体 ID 或基础状态 Tag 定义)
*   `CoreUObject`
*   `Engine` (Subsystems)

## Usage Notes/Guidelines

*   游戏逻辑（如 `Flow`）应将完成的 `FSyOperation` 提交给 `USyStateManager` 的 `RecordOperation` 接口。
*   `USyStateComponent` 应在初始化时获取 `USyStateManager` 实例，并订阅 `OnStateModificationRecorded` 委托（或定期查询）。
*   `FSyStateModificationRecord` 的设计应确保包含足够的信息供 `USyStateComponent` 重建或应用状态变更。

## Future Considerations

*   实现更复杂的记录存储和查询（如按时间范围、按操作类型过滤）。
*   添加记录持久化或网络同步支持（如果需要）。
*   考虑实现操作的撤销/重做机制（需要更复杂的记录结构）。
# SyCombat - GAS Bridge 模块

## 解决的问题
在多人 + 客户端预测场景下，任务/关卡/AI 等系统经常需要更新“死亡/阶段/眩晕/霸体”等关键状态，而这些状态又必须影响 GAS Ability 的可用性与分支。

本模块将 **SyCore StateFacade** 的写入请求映射到 ASC：
- **Persistent**：优先使用 `GameplayEffect`（GrantedTags），复制/回滚友好
- **Temporary**：优先使用 `LooseGameplayTags`，即时反馈（可预测）

## 主要类
- `USyGASStateBackendComponent`：实现 `USyStateBackendBaseComponent`，由 Facade 自动发现并按优先级调度
- `USyStateToGASMapping`：SyState -> GAS Tag/Effect 的映射 DataAsset

## 推荐实践（AbilitySet 集成）
建议采用 **“能力预授予 + Tag Gate”**：

- Ability 在 AbilitySet 中提前授予
- Ability 自身通过 `ActivationRequiredTags/BlockedTags` 决定可用性
- 状态变化通过 GAS Bridge 映射为 GE/GrantedTags，从而自动驱动能力开关


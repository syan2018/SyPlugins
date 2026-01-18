# SyCombat - 战斗管线（核心编排层）

## 定位
`SyCombat` 是 **“流程编排为核心”** 的战斗管线插件：负责把输入/技能/判定/结算/表现等信息流收口到统一管线内。

- **不直接依赖 GAS**：所有重度逻辑与具体实现（Lyra/GAS/自研战斗）都应放在 Adapter/Impl 中。
- **与 SyCore 的关系**：坚持 `USyEntityComponent` 为统一入口；状态写入建议走 `USyEntityStateFacadeComponent`（唯一入口）。

## 当前内容（原型骨架）
- `USyCombatEntityComponent`：战斗实体 facet（基于 `USyEntityComponent` 获取 EntityId）
- `USyCombatPipelineComponent`：管线中枢骨架（事件：`RequestAction` / `ReportHit`）

## GAS Bridge（官方组件）
SyCombat 插件内置 **GAS Bridge 模块**（`SyGASBridge`），用于把 SyCore StateFacade 的写入映射到 ASC：
- Persistent：GameplayEffect / GrantedTags（复制/回滚友好）
- Temporary：LooseGameplayTags（即时/可预测）

详情参见 `SyCombat/README_GASBridge.md`。

## 后续接入建议
- 如果项目使用 GAS/Lyra：使用 `SyCombatLyraAdapter` 把 `ActionTag` 映射到 `GameplayEventTag` 驱动 Ability（预测友好）。
- 集成测试流程见：`Docs/SyCombat_Integration_Test.md`


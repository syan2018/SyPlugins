# SyPlugins 当前状态与计划对比（2026-01）

> 本文档描述 **当前实现状态**，并与初级规划文档进行对比：
> - `Docs/Module/Combat/SyPlugins_Adaptation_Plan.md`
> - `Docs/Module/Combat/SyCombat_Design.md`
> - `Docs/Module/Combat/SyStateSystem_Refactoring_Design.md`

## 1. 总览（当前架构）

### 1.1 状态系统（State）
- **唯一入口**：`USyStateComponent`
- **后端模式**：`USyStateBackendBase`（子对象）
- **标准化配置**：`USyStateProfile`（DefaultInitData + BackendTypes/BackendInstances）
- **已实现后端**
  - `USyGenericStateBackend`：通用 KV 状态
  - `USyGASStateBackend`：GAS Bridge（SyCombat 内 `SyGASBridge` 模块）

### 1.2 战斗系统（SyCombat）
- **核心组件**：`USyCombatComponent`  
  合并 Pipeline + InputBuffer + ResolutionChain，减少组件数量。
- **接口层**：`ISyCombatAbilityDriver` / `ISyCombatProcessor` / `ISyCombatPresentationBridge`
- **Lyra 适配**：`SyCombatLyraAdapter`
  - `USyCombatGASAbilityDriverComponent` 监听 `USyCombatComponent::OnActionRequested`
  - Processor 示例：BuildSpec / ApplySpec / DebugTrace

### 1.3 GAS Bridge
- **模块位置**：`SyCombat/Source/SyGASBridge`
- **核心类**：`USyGASStateBackend`  
  通过 `USyStateToGASMapping` 将 SyState 写入映射为 ASC Tag/GE。

---

## 2. 快速接入流程（State + Combat）

### 2.1 State 接入（推荐）
1. 创建 `USyStateProfile` 资产：配置 `DefaultInitData`
2. 在 Profile 中配置 `BackendTypes`（或 `BackendInstances`）
3. 在角色上挂 `USyStateComponent` 并设置 `StateProfile`
4. 写入统一走 `USyStateComponent::ApplyStateChange`

### 2.2 Combat 接入（Lyra/GAS）
1. 角色挂 `USyCombatComponent` + `USyCombatGASAbilityDriverComponent`
2. 在 Driver 上配置 `ActionToGameplayEventTag`
3. Ability 监听对应 GameplayEventTag 并设置 Tag Gate
4. 通过 `USyCombatComponent::RequestAction` 触发

---

## 3. 与初级规划的对比

### 3.1 `SyStateSystem_Refactoring_Design.md`（联邦状态架构）
| 规划条目 | 当前状态 | 说明 |
| --- | --- | --- |
| Facade + Backend 架构 | ✅ 已实现 | `USyStateComponent` + `USyStateBackendBase` |
| 通用后端（Generic） | ✅ 已实现 | `USyGenericStateBackend` |
| GAS 后端 | ✅ 已实现 | `USyGASStateBackend` |
| Numeric/Reflection 后端 | ❌ 未实现 | 仍在规划 |
| Tag 前缀路由/更细粒度 API | ⚠️ 仅基础路由 | 目前为遍历式后端优先级 |

### 3.2 `SyCombat_Design.md`（SCPA）
| 规划条目 | 当前状态 | 说明 |
| --- | --- | --- |
| Combat Pipeline（请求/命中/结算） | ✅ 已实现 | 合并到 `USyCombatComponent` |
| Input Buffer | ✅ 已实现 | 轻量缓冲 |
| Resolution Chain | ✅ 已实现 | Processor 链 |
| Presentation Bridge | ✅ 已实现 | MessageBus / GameplayCue 适配 |
| AI / Weapon / Perception | ❌ 未实现 | 仍在规划 |

### 3.3 `SyPlugins_Adaptation_Plan.md`（SyCore Processing）
| 规划条目 | 当前状态 | 说明 |
| --- | --- | --- |
| `FSyOperationRequest` | ❌ 未实现 | 仍在规划 |
| `ISyOperationProcessor` + Subsystem | ❌ 未实现 | 目前仅 SyCombat 内部链 |
| SyCore Processing 子模块 | ❌ 未实现 | 仍在规划 |

---

## 4. 关键决策变更（与早期规划不同）
1. **State 入口收敛**：取消 Facade 组件，`USyStateComponent` 作为唯一入口。
2. **后端子对象化**：后端以 `UObject` 子对象持有，允许 Profile 标准化配置。
3. **Combat 组件收敛**：`USyCombatComponent` 合并 Pipeline + Buffer + Chain，减少组件数量与复杂度。
4. **GAS Bridge 内置**：`SyGASBridge` 作为 SyCombat 内部模块，而非独立插件。
5. **类型命名收敛**：`SyStateTypes` 替代旧的 `SyStateFacadeTypes`。

---

## 5. 现阶段可用能力清单
- 状态写入统一入口（StateComponent）
- GAS 关键状态映射（GE/Tag）
- 战斗 ActionRequest -> GameplayEvent 预测触发
- 基础 ResolutionChain（Processor 顺序）
- Presentation Bridge（MessageBus / GameplayCue）

---

## 6. 已知缺口（下一阶段）
- SyCore 级别的通用 Processing 子模块（跨系统复用）
- Numeric/Reflection 后端（数值高频/原生属性）
- AI / Weapon / Perception 的标准化接口与管线接入


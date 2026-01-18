# SyCombat 集成测试文档（Lyra/GAS）

> 目标：验证 SyCore StateComponent + SyCombat + SyCombatLyraAdapter 的 **端到端链路** 是否正常工作，覆盖“状态写入 -> ASC Tag/GE -> Ability Gate -> 预测触发 -> 结算链”。

## 0. 前置条件
启用以下插件/模块：
- `SyCore`
- `SyCombat`（包含 `SyGASBridge` 模块）
- `SyCombatLyraAdapter`
- `GameplayAbilities` / `GameplayTasks` / `GameplayTags`（项目本身）

## 1. 组件挂载清单（角色或敌人 Actor）
在目标 Actor（例如 Lyra 角色）上添加：

**核心入口**
- `USyEntityComponent`
- `USyStateComponent`（唯一入口）

**标准化配置（推荐）**
- 在 `USyStateComponent.StateProfile` 中配置：
  - `DefaultInitData`
  - `BackendTypes` / `BackendInstances`

**状态后端（子对象）**
- 可通过 `USyStateComponent.Backends` 手动配置（需要自定义 Mapping 时推荐）
- `USyGenericStateBackend`（需要通用状态时）
- `USyGASStateBackend`（GAS Bridge）

**GAS Bridge 配置**
- 在 `USyGASStateBackend` 上配置 `USyStateToGASMapping` 资产（推荐用 Profile 的 BackendInstances 来配置）

**SyCombat 核心**
- `USyCombatComponent`

**Lyra/GAS 适配**
- `USyCombatGASAbilityDriverComponent`
- `USyCombatLyraDemoSetupComponent`（原型演示用，可选）

**表现桥（任选其一）**
- `USyCombatMessageBridgeComponent`（走 SyMessageBus）
- `USyCombatLyraPresentationBridgeComponent`（走 GameplayCue）

## 2. 创建映射资产（State -> GAS）

1) 新建 `USyStateToGASMapping` DataAsset  
2) 配置至少一条映射，例如：  

- `SyStateTag`: `Sy.State.Life.Dead`  
- `GameplayTag`: `State.Dead`  
- `PersistentTagEffect`: `GE_State_Dead`  
- `bAllowLooseTagPrediction`: `true`  

> 说明：Persistent 写入默认使用 GE（复制/回滚友好）。没有 GE 时会退化为 loose tag（不建议用于关键状态）。

## 3. 创建 GameplayEffect（GrantedTags）

创建 `GE_State_Dead`（或你自己的命名）：  
- `GrantedTags` 包含 `State.Dead`  
  
该 GE 将由 `USyGASStateBackend` 在服务器权威侧应用/移除。

## 4. AbilitySet / Ability 配置

在 AbilitySet 中：
- 预授予核心能力  
- 在 Ability 上配置 `ActivationRequiredTags` / `ActivationBlockedTags`  
  - 例如：若 `State.Dead` 存在则 **阻止** 某些能力  

Ability 触发方式：
- 让 Ability 监听 `GameplayEventTag`  
- 在 `USyCombatGASAbilityDriverComponent.ActionToGameplayEventTag` 中配置映射

## 5. 测试步骤（推荐 PIE 双客户端）

### 5.1 状态写入 -> ASC Tag/GE
**目标**：验证 StateComponent 写入能正确影响 ASC 状态。

在运行时调用：
1. 创建 `FSyStateChangeRequest`  
   - `Scope = Entity`  
   - `Layer = Persistent`  
   - `StateTag = Sy.State.Life.Dead`  
   - `Value = FSyBoolValue(true)`  
2. 调用 `USyStateComponent::ApplyStateChange`

预期结果：
- 服务器侧 ASC 应持有 `State.Dead`（通过 GE）
- 客户端可收到复制/回滚后的正确标签状态
- 若未配置映射或未启用 GAS Bridge：日志会提示拒绝 EntityScope 持久写入（这是刻意设计）

### 5.2 临时状态 -> LooseTag（即时反馈）
**目标**：验证临时写入是否走 LooseTags。

1. 构建 `FSyStateChangeRequest`  
   - `Scope = Entity`  
   - `Layer = Temporary`  
   - `StateTag = Sy.State.Combat.SuperArmor`  
   - `Value = FSyBoolValue(true)`  
2. 调用 `ApplyStateChange`

预期结果：
- ASC 立即获得对应 GameplayTag（LooseTag）
- 服务器权威状态随后对齐（如果也有 Persistent 版本）

### 5.3 ActionTag -> GameplayEvent -> Ability
**目标**：验证 ActionRequest 驱动 GAS 预测触发。

1. 在 `USyCombatLyraDemoSetupComponent.DefaultActionToEvent` 中添加映射  
   - `Sy.Combat.Action.LightAttack` -> `Event.Combat.LightAttack`  
2. 在 Ability 内监听 `Event.Combat.LightAttack`  
3. 调用 `USyCombatComponent::RequestAction`

预期结果：
- 客户端立即触发 Ability（预测）
- 服务器权威执行后保持一致

### 5.4 结算链（Processor 执行顺序）
**目标**：验证 Processor 注册与执行。

默认链：  
1) `SyLyraBuildSpecProcessor`  
2) `SyLyraApplySpecProcessor`  
3) `SyLyraDebugTraceProcessor`

预期结果：
- 日志输出顺序与优先级一致（优先级高先执行）

## 6. 常见问题排查

- **EntityScope + Persistent 被拒绝**  
  - 检查：是否挂载 `USyGASStateBackend`  
  - 检查：是否配置 `USyStateToGASMapping`  
  - 检查：是否为该 `SyStateTag` 提供映射

- **Ability 没触发**  
  - 检查：`ActionTag -> GameplayEventTag` 映射  
  - 检查：Ability 是否监听该 EventTag  
  - 检查：ASC 是否存在 / 角色是否实现 `IAbilitySystemInterface`

- **状态不同步**  
  - 检查：GE 是否只在服务器侧应用  
  - 检查：客户端是否使用 LooseTag 作为即时反馈  

## 7. 建议的最小演示清单
- 1 个状态：`Sy.State.Life.Dead` → `State.Dead`（GE）
- 1 个 Ability：监听 `Event.Combat.LightAttack`（可预测）
- 1 个 Action：`Sy.Combat.Action.LightAttack`


# SyCombat × Lyra 集成验证计划

> **目标**：在不修改 Lyra 源码的前提下，将 SyCombat 管线嵌入 Lyra 现有战斗流程，验证管线本身的可靠性。  
> **原则**：组件挂载 + Adapter 桥接，逐步接管、逐层验证。

---

## 1. Lyra 现有战斗架构分析

### 1.1 信息流全景

```
[InputAction]
  → [HeroComponent] InputTag
    → [ASC] ProcessAbilityInput → TryActivateAbility
      → [GA_RangedWeapon] TraceBullets → TargetData
        → [Server] Confirm → Apply GE(DamageExecution)
          → [HealthSet] BaseDamage → Distance/Material/Team → -Health
            → [HealthComponent] OnOutOfHealth → StartDeath
              → [GA_Death] 死亡表现
```

### 1.2 五层架构

| 层级 | 核心类 | 职责 |
|---|---|---|
| **输入层** | `ULyraHeroComponent`, `ULyraInputComponent` | Enhanced Input → InputTag → ASC 帧末批处理 |
| **能力层** | `ULyraGameplayAbility`, `ULyraAbilitySystemComponent` | ActivationPolicy/Group, TagRelationshipMapping, AbilitySet 授予 |
| **武器层** | `ULyraEquipmentManagerComponent`, `ULyraRangedWeaponInstance` | 装备管理、AbilitySet 授予、距离/材质衰减（ILyraAbilitySourceInterface） |
| **结算层** | `ULyraDamageExecution`, `ULyraCombatSet` | BaseDamage 捕获 → 队伍校验 → 衰减计算 → 输出 Damage meta-attribute |
| **生命层** | `ULyraHealthSet`, `ULyraHealthComponent` | Damage → -Health, FLyraVerbMessage 广播, 死亡状态机 |

### 1.3 关键设计特征

- **InputTag 驱动**：输入不直接调用能力，而是通过 `ASC.AbilityInputTagPressed(Tag)` 进入帧末队列
- **ActivationGroup 互斥**：`Exclusive_Replaceable` / `Exclusive_Blocking` 控制能力间的排斥关系
- **FLyraGameplayEffectContext**：扩展了 CartridgeID、AbilitySource、PhysicalMaterial 等射击上下文
- **FLyraVerbMessage**：通过 `GameplayMessageSubsystem` 广播伤害消息（解耦 UI/反馈）

---

## 2. SyCombat 管线 vs Lyra 衔接分析

### 2.1 信息流对位

| 战斗阶段 | Lyra 方案 | SyCombat 方案 | 冲突程度 |
|---|---|---|---|
| **输入采集** | Enhanced Input → InputTag → ASC.ProcessAbilityInput | ActionTag → RequestAction / BufferAction | 低（可并行） |
| **能力激活** | ASC.TryActivateAbility（InputTag 直接驱动） | OnActionRequested → AbilityDriver → SendGameplayEvent | 低（Event 与 InputTag 是 GAS 两种共存的激活方式） |
| **命中判定** | GA_RangedWeapon 内部 Trace → TargetData | ReportHit(FSyCombatHitContext) 回灌 | 中（Lyra 判定逻辑深嵌在 Ability 中） |
| **伤害结算** | LyraDamageExecution（GE ExecutionCalc） | ProcessorChain（BuildSpec → ApplySpec） | 高（两套结算管线） |
| **状态管理** | ASC Tags/GE 直接操作 | SyState → GASBridge → ASC | 中（桥接层可叠加但增加复杂度） |
| **表现反馈** | GameplayMessageSubsystem + GameplayCue | PresentationBridge → MessageBus / GameplayCue | 低（可共存） |

### 2.2 核心矛盾

1. **输入通路分裂**：Lyra `InputTag → ASC.ProcessAbilityInput` 和 SyCombat `ActionTag → GameplayEvent` 是两条平行通路
2. **结算链冲突**：Lyra 的 `ExecutionCalculation` vs SyCombat 的 `ProcessorChain`，不能简单叠加
3. **Trace 归属**：Lyra 将射击检测嵌入 `GA_RangedWeapon` 内部，SyCombat 期望 HitContext 作为外部输入

---

## 3. 分阶段实施方案

### Phase 0：基础挂载（验证组件共存）

**目标**：SyEntity + SyCombat 组件与 Lyra 角色共存，不打断现有功能。

**步骤**：
1. 在 Lyra 角色蓝图上挂载：
   - `USyEntityComponent`
   - `USyStateComponent`（配置 StateProfile + GASBridge Backend）
   - `USyCombatComponent`
   - `USyCombatGASAbilityDriverComponent`
   - `USyCombatMessageBridgeComponent`（表现桥）
2. 创建 `USyStateToGASMapping` DataAsset，配置最小映射：
   - `Sy.State.Life.Dead` → `State.Dead`（配 GE）
3. 运行游戏，确认 Lyra 原有战斗流程完全不受影响

**验证点**：组件初始化无报错，Lyra 射击/伤害/死亡链路正常。  
**预估工时**：0.5 天

---

### Phase 1：旁路监听（验证管线信息采集）

**目标**：SyCombat 管线作为观察者旁路采集 Lyra 战斗事件，不介入实际流程。

**新增组件**：`USyCombatLyraObserverComponent`（放入 SyCombatLyraAdapter）

```cpp
UCLASS(Blueprintable, ClassGroup=(SyEntity), meta=(BlueprintSpawnableComponent))
class USyCombatLyraObserverComponent : public UActorComponent, public ISyComponentInterface
{
    GENERATED_BODY()
public:
    // 监听 ASC 能力激活/结束
    // 监听 ULyraHealthSet::OnHealthChanged / OnOutOfHealth
    // 翻译为 FSyCombatHitContext / FSyCombatActionRequest
    // 调用 USyCombatComponent::ReportHit() 回灌（纯记录，不触发结算）
};
```

**具体逻辑**：
1. `OnSyComponentInitialized` 中找到 ASC 和 `ULyraHealthSet`，绑定委托
2. 监听 ASC 的 `AbilityActivatedCallbacks`，记录每次能力触发
3. `OnOutOfHealth` 触发时翻译为 SyState 写入（`Sy.State.Life.Dead`），验证 GASBridge 映射

**验证点**：
- SyCombat 能正确采集 Lyra 的射击和伤害事件
- GASBridge State → ASC Tag 映射正确
- 日志输出与 Lyra 原生行为一致

**预估工时**：1-2 天

---

### Phase 2：输入接管（验证 ActionTag → Ability 通路）

**目标**：将"开火"输入拦截到 SyCombat 管线，由管线通过 GameplayEvent 驱动 Ability 激活。

**新增组件**：`USyCombatLyraInputBridgeComponent`

```cpp
UCLASS(Blueprintable)
class USyCombatLyraInputBridgeComponent : public UActorComponent, public ISyComponentInterface
{
    GENERATED_BODY()
public:
    // InputTag → ActionTag 映射配置
    // InputTag.Ability.Fire → Sy.Combat.Action.Fire
    
    // 监听 HeroComponent::NAME_BindInputsNow 后额外注册 InputAction Handler
    // 当 Fire 按下时调用 USyCombatComponent::RequestAction()
};
```

**分步策略**：

- **Phase 2a（保守）**：并行运行——SyCombat 和 Lyra 同时工作，对比结果
- **Phase 2b（进取）**：SyCombat 接管——ASC 添加 `TAG_Gameplay_AbilityInputBlocked` 阻止原生通路，由 GameplayEvent 唯一驱动

**验证点**：
- InputBuffer 正确缓冲连续输入（MaxBuffer=8, Expiration=0.35s）
- ActionTag → GameplayEventTag 正确激活 Ability
- 射击 Ability 行为与原生 InputTag 激活完全一致
- 客户端预测正常

**预估工时**：1 天

---

### Phase 3：结算链桥接（验证 ProcessorChain）

**目标**：SyCombat ProcessorChain 能"观察并增强" Lyra 伤害结算。

**策略**：不替换 `ULyraDamageExecution`，在其前后插入 SyCombat 处理节点。

**新增 Processor**：

```cpp
// Priority=300: 从 LyraGameplayEffectContext 提取信息
class USyLyraExtractDamageContextProcessor : public UObject, public ISyCombatProcessorInterface
{
    // 提取 CartridgeID, HitResult, AbilitySource, Distance, PhysicalMaterial
    // 填入 FSyCombatOperationRequest.ContextTags / NumericModifiers
};

// Priority=200: SyCombat 侧数值加工（Buff/减伤/暴击等）
class USyLyraCombatModifierProcessor : public UObject, public ISyCombatProcessorInterface
{
    // 读取 SyState Buff/Debuff，修改 NumericModifiers
};

// Priority=100: 将结果应用回 GAS
class USyLyraApplyDamageProcessor : public UObject, public ISyCombatProcessorInterface
{
    // 加工后 NumericModifiers 设回 CombatSet.BaseDamage
    // 或直接构建 GE Spec 应用
};
```

**验证点**：
- ProcessorChain 按 Priority 排序执行
- 伤害数值经链路加工后结果正确
- DebugTraceProcessor 输出完整数值变化链路

**预估工时**：2-3 天

---

### Phase 4：全链路贯通（端到端验证）

**目标**：验证完整的 SyCombat 管线闭环。

```
[按下开火]
  → [InputBridge] 翻译为 ActionTag
    → [CombatComponent] RequestAction + InputBuffer
      → [GASAbilityDriver] ActionTag → GameplayEventTag → ASC
        → [GA_RangedWeapon] Trace（Lyra 原生）
          → [Observer] HitResult → ReportHit(HitContext)
            → [ProcessorChain] 数值加工
              → [GAS] Apply GE → HealthSet
                → [GASBridge] Health→0 → SyState(Dead) → ASC Tag
                  → [GA_Death] 死亡表现
```

**预估工时**：1 天集成 + 1 天测试

---

## 4. 风险矩阵

| 风险 | 影响 | 缓解措施 |
|---|---|---|
| 组件初始化时序冲突 | 高 | SyCombat 在 `ISyComponentInterface::OnSyComponentInitialized` 初始化（Phase=Functional），确保 Lyra PawnExtension 完成后再绑定 |
| GameplayEvent 预测行为与 InputTag 不一致 | 中 | Phase 2a 并行对比验证 |
| 双重伤害结算 | 高 | Phase 3 做"观察增强"（不替换 Execution），或"完全接管"（替换 GE），不能两路同时写入 Health |
| GASBridge 映射与 Lyra 已有 Tag 冲突 | 低 | `Sy.State.*` 命名空间隔离 |

---

## 5. 推荐的最小验证集

若以最小成本验证管线可靠性，聚焦 **Phase 0 + Phase 1 + Phase 2a**：

1. 挂载组件（0.5 天）
2. Observer 旁路监听（1-2 天）
3. 输入并行验证（1 天）

这三步可验证：
- 组件生命周期管理是否稳定
- ActionTag → GameplayEvent → Ability 驱动链路是否可靠
- InputBuffer 缓冲/过期/消费逻辑是否正确
- GASBridge 状态映射是否正确

**不需要触碰 Lyra 伤害结算**（风险最高部分），管线基础验证通过后再推进 Phase 3-4。

---

## 附录：组件挂载清单（角色 Actor）

| 组件 | 来源 | 职责 |
|---|---|---|
| `USyEntityComponent` | SyCore | 实体标识与统一管理 |
| `USyStateComponent` | SyCore | 状态管理唯一入口 |
| `USyGASStateBackend` | SyGASBridge | SyState → ASC Tag/GE 映射（作为 StateComponent 后端） |
| `USyCombatComponent` | SyCombat | 管线核心（Pipeline + InputBuffer + ResolutionChain） |
| `USyCombatGASAbilityDriverComponent` | SyCombatLyraAdapter | ActionTag → GameplayEventTag 驱动 |
| `USyCombatMessageBridgeComponent` | SyCombat | 表现事件 → SyMessageBus |
| `USyCombatLyraObserverComponent` | SyCombatLyraAdapter（新增） | 旁路监听 Lyra 战斗事件 |
| `USyCombatLyraInputBridgeComponent` | SyCombatLyraAdapter（新增） | InputTag → ActionTag 桥接 |

# SyPlugins Architecture Adaptation Plan for SyCombat
# SyPlugins 针对战斗系统 (SyCombat) 的架构适配规划

> **Status**: Draft
> **Date**: 2025-11-03
> **Target**: Enhance SyCore to support the SyCombat Pipeline Architecture (SCPA)

## 1. 背景与目标

根据 [SyCombat Design](SyCombat_Design.md) (SCPA) 的规划，我们需要在 `SyPlugins` 体系中实现一套高可扩展的战斗管线。
当前 `SyCore` (State + Message) 的架构在“意图(Request)”到“结果(Result)”之间缺乏标准化的 **中间处理层 (Middleware/Processor Layer)**。

本规划旨在定义如何升级 `SyCore` 基础设施，使其能够原生支持 SCPA 所需的 **Resolution Chain (结算链)** 和 **Operation Processing (操作处理)** 模式，从而避免在 `SyCombat` 中重复造轮子，并确保该模式可复用于其他系统（如任务奖励结算、对话属性检定等）。

---

## 2. 核心概念升级 (Core Concept Upgrade)

我们需要在 Message (触发) 和 State (结果) 之间，引入 **Processing (处理)** 阶段。

| 阶段 | 概念 | 对应 SCPA 概念 | 当前 SyCore 状态 | 升级计划 |
| :--- | :--- | :--- | :--- | :--- |
| **1. Trigger** | **Message / Event** | Input / Action Request | ✅ SyMessageBus | 保持现状，作为触发源 |
| **2. Intent** | **Operation Request** | Raw Operation | ❌ (缺失) | 新增 `FSyOperationRequest` |
| **3. Process** | **Processor Pipeline** | Resolution Chain | ❌ (缺失) | 新增 `ISyOperationProcessor` 及其调度器 |
| **4. Result** | **Operation** | Final Operation | ✅ FSyOperation | 保持现状，作为最终提交物 |
| **5. Apply** | **State Change** | Final Application | ✅ StateManager | 保持现状，负责持久化 |

---

## 3. SyCore 架构变更规划

### 3.1 新增模块: `SyCore/Processing`

在 `SyCore` 中新增子模块 `Processing`，提供通用的责任链模式实现。

#### 3.1.1 `FSyOperationRequest` (意图)
不同于最终的 `FSyOperation` (那是不可变的日志)，`Request` 是**可变**的，用于在管线中传递和修改。

```cpp
struct FSyOperationRequest
{
    // 原始来源
    AActor* Instigator;
    AActor* Target;
    const UObject* SourceObject; // 武器/技能/任务对象
    
    // 待处理的数据 (Key-Value)
    // 例如: { "Damage.Base": 100, "Heal.Bonus": 20 }
    FSyStateParameterSet PendingModifiers;
    
    // 上下文标签 (用于逻辑判断)
    // 例如: { "Effect.Fire", "Combat.Melee", "Quest.Reward" }
    FGameplayTagContainer ContextTags;
    
    // 转换为最终 Operation 的方法
    FSyOperation ToFinalOperation() const;
};
```

#### 3.1.2 `ISyOperationProcessor` (处理器接口)
定义对 Request 进行处理的标准接口。

```cpp
class ISyOperationProcessor : public IInterface
{
    // 核心处理函数
    virtual void ProcessRequest(FSyOperationRequest& InOutRequest, const FSyProcessContext& Context) = 0;
    
    // 优先级 (决定执行顺序: Defense -> Resistance -> Shield -> HP)
    virtual int32 GetProcessingPriority() const = 0;
};
```

#### 3.1.3 `USyPipelineSubsystem` (管线管理器)
负责维护和执行 Processor Chain。

*   **注册机制**: 允许不同模块注册 Processor (例如 `SyCombat` 注册防御计算，`SyQuest` 注册奖励加成)。
*   **执行流**:
    ```cpp
    FSyOperation FinalOp = Pipeline->Process(RawRequest);
    StateManager->RecordOperation(FinalOp);
    ```

---

## 4. SyCombat 适配落地 (Implementation Strategy)

基于升级后的 `SyCore`，`SyCombat` 的实现将变得非常轻量化。

### 4.1 战斗流程重构

1.  **Input**: 
    *   `SyCombatPipeline` 接收输入，决定发动攻击。
2.  **Builder**: 
    *   构建 `FSyOperationRequest`。
    *   从 `CombatSource` (武器) 读取基础伤害 -> 填入 `PendingModifiers`。
    *   添加标签 `Combat.Damage.Physical`。
3.  **Process (SyCore 能力)**:
    *   调用 `PipelineSubsystem->Process(Request)`。
    *   **Processor A (Stats)**: 读取攻击者属性(Strength)，增加 Damage。
    *   **Processor B (Defense)**: 读取受击者防御(Defense)，减少 Damage。
    *   **Processor C (Reaction)**: 发现是火属性且目标有油，追加 `Effect.Burning` 到 Modifiers。
4.  **Commit (SyCore 能力)**:
    *   得到最终 `FSyOperation`。
    *   提交给 `StateManager`。

### 4.2 优势

*   **复用性**: 这套 Processor 机制完全可以用于 **任务奖励结算** (Processor: VIP加成、双倍活动) 或 **对话属性检定** (Processor: 魅力值修正)。
*   **解耦**: 战斗模块不需要知道“防御力”具体怎么算的，只需要注册一个 DefenseProcessor。
*   **调试**: 可以在 Pipeline 中插入 DebugProcessor，打印每一步数值变化 (Damage: 100 -> 120 -> 80)。

---

## 5. 待办事项 (Action Items)

### Phase 1: SyCore 增强
- [ ] 在 `SyCore` 中创建 `Processing` 目录。
- [ ] 定义 `FSyOperationRequest` 结构体。
- [ ] 定义 `ISyOperationProcessor` 接口。
- [ ] 实现基础的 Pipeline 调度器 (简单的 TArray 遍历)。

### Phase 2: SyCombat 原型验证
- [ ] 创建一个简单的 `DamageProcessor` (硬编码减伤)。
- [ ] 在 `SyPluginsImpl` 中演示：Input -> Request -> Process -> State Change 的完整链路。

### Phase 3: 工具链支持
- [ ] (可选) 为 Processor Pipeline 增加可视化调试工具 (Visual Logger 支持)。

---

**总结**: 
通过在 `SyCore` 中固化 "Request -> Process -> Result" 模式，我们不仅解决了 SyCombat 的“别扭”感，还为 SyPlugins 提供了一套通用的**复杂逻辑处理框架**。

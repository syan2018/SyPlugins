# SyStateSystem Refactoring Design: Federated Architecture
# SyStateSystem 重构设计：联邦状态架构

> **Status**: Draft
> **Date**: 2025-11-03
> **Target**: Resolve the "Jack of all trades, master of none" issue in the current State System.

## 1. 问题陈述 (Problem Statement)

当前的 `SyStateSystem` 基于 `TMap<Tag, Metadata>` 和 `FInstancedStruct` 构建，本质上是一个**通用型键值对存储 (Generic K-V Store)**。
这种设计虽然极其灵活（适合任务、剧情、物品），但在面对 **战斗系统 (Combat System)** 需求时暴露出了核心缺陷：

1.  **缺乏数值计算能力**: 仅支持“覆盖(Override)”操作，无法原生处理“扣血(Add -10)”或“攻击力加成(Multiply 1.2)”。
2.  **性能隐患**: 频繁的 `TMap` 查找和 `InstancedStruct` 内存分配不适合高频战斗计算。
3.  **GAS 兼容性差**: 无法直接映射到 Unreal GAS 的 `AttributeSet`，导致未来集成困难。

## 2. 核心设计理念 (Core Concept)

引入 **联邦状态架构 (Federated State Architecture)**。
`SyStateComponent` 不再是一个单一的容器，而是一个 **门面 (Facade)**，它管理着多个 **后端 (Backends)**。

不同的状态数据由最适合它的后端来管理：
*   **Attributes (HP/MP)** -> 交给 **Numeric Backend** (或 GAS)。
*   **Quest/World States** -> 交给 **Generic Backend** (即当前的实现)。
*   **Native Properties** -> 交给 **Reflection Backend** (直接读写 C++ 变量)。

---

## 3. 架构设计 (Architecture)

### 3.1 核心接口层 (The Interface Layer)

定义 `ISyStateBackend` 接口，抹平不同存储方式的差异。

```cpp
class ISyStateBackend
{
public:
    // 查询能力
    virtual bool ContainsTag(const FGameplayTag& Tag) const = 0;
    
    // 数值操作 (战斗用)
    virtual float GetNumericValue(const FGameplayTag& Tag, float DefaultValue = 0.0f) const { return DefaultValue; }
    virtual void ApplyNumericChange(const FGameplayTag& Tag, float Value, EModifierOp Op) {}

    // 通用操作 (剧情用)
    virtual const FInstancedStruct* GetGenericValue(const FGameplayTag& Tag) const { return nullptr; }
    virtual void SetGenericValue(const FGameplayTag& Tag, const FInstancedStruct& Value) {}
};
```

### 3.2 状态组件改造 (SyStateComponent Refactoring)

`SyStateComponent` 将持有后端列表，并负责路由。

```cpp
class USyStateComponent
{
    // 后端列表 (按优先级排序)
    TArray<TScriptInterface<ISyStateBackend>> Backends;

public:
    // 路由逻辑
    float GetAttribute(FGameplayTag Tag) {
        // 策略A: 遍历所有后端，返回第一个能处理的
        // 策略B: 根据 Tag 前缀路由 (如 "Attr.*" -> Backend[0])
        return FindBackendFor(Tag)->GetNumericValue(Tag);
    }
};
```

---

## 4. 数据结构升级 (Data Structure Upgrade)

### 4.1 引入算术操作 (Arithmetic Operations)

升级 `FSyOperation` 及其 Modifier，使其具备数学语义。

```cpp
UENUM()
enum class ESyModifierOp : uint8
{
    Override,   // 直接设置 (Set)
    Add,        // 加法 (用于 伤害/治疗)
    Multiply    // 乘法 (用于 百分比Buff)
};

USTRUCT()
struct FSyStateModifierItem
{
    UPROPERTY()
    ESyModifierOp OpType = ESyModifierOp::Override;

    // 数值通道 (战斗高性能通道)
    UPROPERTY()
    float NumericValue = 0.0f;

    // 结构体通道 (通用低性能通道)
    UPROPERTY()
    FInstancedStruct GenericValue;
};
```

---

## 5. 实施路线图 (Implementation Roadmap)

### Phase 1: 接口抽象 (Interface Abstraction)
1.  定义 `ISyStateBackend`。
2.  将现有的 `FSyStateCategories` 封装为 `FSyGenericStateBackend`。
3.  让 `SyStateComponent` 使用接口访问数据，而不是直接操作 Map。

### Phase 2: 数值后端 (Numeric Backend)
1.  实现 `FSyNumericStateBackend`，内部使用 `TMap<Tag, float>` (简单版) 或 `AttributeSet` (GAS版)。
2.  在 `SyOperation` 中支持 `ESyModifierOp`。
3.  实现基础的“扣血”逻辑测试：发送 Operation(Add -10) -> Backend 更新 -> 数值变动。

### Phase 3: 路由系统 (Routing System)
1.  实现基于 Tag 的路由策略。
    *   `Attribute.*` -> Numeric Backend
    *   `State.*` -> Generic Backend
2.  在 Editor 中提供配置界面，允许开发者指定 Tag 分段对应的后端。

---

## 6. 对比优势 (Benefits)

| 特性 | 旧架构 | 新架构 (联邦式) |
| :--- | :--- | :--- |
| **数值计算** | 不支持 (需先读再写) | **原生支持 (Add/Mul)** |
| **性能** | 低 (结构体拷贝) | **高 (浮点运算)** |
| **GAS集成** | 困难 | **平滑 (只需写一个 Adapter Backend)** |
| **灵活性** | 高 | **高 (保留了通用存储能力)** |
| **代码复杂度** | 低 | 中 (增加了抽象层) |

这一重构将使 SyStateSystem 真正成为一个**通用的、生产级**的状态管理方案，既能胜任 RPG 复杂的剧情标记，也能支撑 ACT 高频的战斗数值。

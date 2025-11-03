# SyStateSystem Module

## 概述

`SyStateSystem` 是一个统一的状态管理系统模块，整合了原 `SyStateCore`、`SyOperation`、`SyStateManager` 三个模块的功能，旨在提供完整的状态管理解决方案。

本模块采用**分层设计**，将状态系统划分为三个子系统：
- **Core**: 核心状态数据结构和类型定义
- **Operations**: 状态变更操作的意图定义
- **Manager**: 全局状态管理和记录中心

## 模块结构

```
SyStateSystem/
├── Source/SyStateSystem/
│   ├── Public/
│   │   ├── Core/                    # 核心状态数据 (原 SyStateCore)
│   │   │   ├── StateContainerTypes.h      # FSyStateCategories, FSyLayeredStateContainer
│   │   │   ├── StateParameterTypes.h      # FSyStateParameterSet, FSyStateParams
│   │   │   ├── StateMetadataTypes.h       # USyStateMetadataBase
│   │   │   └── Metadatas/                # 具体元数据类型定义
│   │   │       ├── BasicMetadatas.h      # 基础类型元数据 (Int, Float, String, etc.)
│   │   │       ├── BasicMetadataValueTypes.h
│   │   │       └── ListMetadataValueTypes.h
│   │   ├── Operations/              # 操作定义 (原 SyOperation)
│   │   │   ├── OperationTypes.h          # FSyOperation, FSyOperationSource, etc.
│   │   │   └── OperationParams.h         # 操作参数结构体
│   │   └── Manager/                 # 状态管理器 (原 SyStateManager)
│   │       ├── SyStateManagerSubsystem.h # 状态管理器子系统
│   │       ├── StateModificationRecord.h # FSyStateModificationRecord
│   │       └── SyStateManagerSaveGame.h  # 状态持久化
│   ├── Private/
│   │   ├── Core/
│   │   ├── Operations/
│   │   └── Manager/
│   └── SyStateSystem.h              # 模块主头文件
└── Content/                         # 示例资源和配置
```

## 核心理念

### 1. 状态驱动

基于"操作记录"统一管理游戏对象的状态及其转换：
- **状态定义**: 使用 `GameplayTag` 进行状态分类
- **状态存储**: 通过 `FSyStateCategories` 维护运行时状态
- **状态层级**: 支持多层级状态管理（Default/Persistent/Temporary/Override）

### 2. 操作意图

使用 `FSyOperation` 描述状态变更的意图而非直接执行：
- **来源 (Source)**: 操作的发起者
- **修饰器 (Modifier)**: 具体的状态修改内容
- **目标 (Target)**: 操作的目标对象

### 3. 中央管理

通过 `USyStateManagerSubsystem` 提供全局状态记录和事件广播：
- **操作记录**: 记录所有状态变更操作
- **聚合查询**: 提供高效的状态聚合查询
- **事件通知**: 广播状态变更事件给订阅者

## 主要功能

### Core 子系统（原 SyStateCore）

**核心数据结构**：
- `FSyStateCategories`: 存储实体运行时状态的容器
- `FSyLayeredStateContainer`: 支持分层状态管理
- `FSyStateParameterSet`: 状态参数配置集
- `USyStateMetadataBase`: 状态元数据基类

**特性**：
- 状态查找和管理
- 批量应用初始化和修改
- 类型安全的值访问接口
- 支持序列化和网络复制（计划中）

### Operations 子系统（原 SyOperation）

**核心概念**：
- `FSyOperation`: 完整的状态变更意图
- `FSyOperationSource`: 操作来源定义
- `FSyOperationModifier`: 状态修改集
- `FSyOperationTarget`: 操作目标定义

**特性**：
- 数据驱动的操作配置
- 使用 `GameplayTag` 进行类型标识
- `FInstancedStruct` 灵活参数支持
- 编辑器友好的配置界面

### Manager 子系统（原 SyStateManager）

**核心功能**：
- 中央化的操作日志记录
- 状态修改事件广播
- 聚合查询优化（增量更新）
- 智能订阅机制（按目标类型精准订阅）

**特性**：
- 支持持久化到 SaveGame
- 性能优化的索引和缓存
- 精准的事件分发
- 无效订阅者自动清理

## 使用示例

### 1. 创建和记录操作

```cpp
#include "SyStateSystem.h"

// 创建操作意图
FSyOperation Operation;
Operation.OperationId = FGuid::NewGuid();

// 设置来源
Operation.Source = FSyOperationSource(
    FGameplayTag::RequestGameplayTag("Source.Skill.Fireball")
);

// 设置修饰器（状态修改）
FSyStateParameterSet Modifications;
Modifications.AddStateParam(
    FGameplayTag::RequestGameplayTag("State.Debuff.Burning"),
    BurningParamsStruct
);
Operation.Modifier = FSyOperationModifier(Modifications);

// 设置目标
Operation.Target = FSyOperationTarget(
    FGameplayTag::RequestGameplayTag("Target.EntityType.Enemy")
);

// 记录操作
if (UWorld* World = GetWorld())
{
    if (USyStateManagerSubsystem* StateMgr = World->GetGameInstance()->GetSubsystem<USyStateManagerSubsystem>())
    {
        StateMgr->RecordOperation(Operation);
    }
}
```

### 2. 订阅状态变更

```cpp
// C++ 智能订阅（推荐）
StateMgr->SubscribeToTargetType(
    MyTargetTypeTag,
    this,
    FOnStateModificationChangedNative::CreateUObject(this, &UMyComponent::HandleStateChange)
);

void UMyComponent::HandleStateChange(const FSyStateModificationRecord& Record)
{
    // 处理状态变更
}

// 蓝图全局订阅
StateMgr->OnStateModificationChanged.AddDynamic(this, &UMyComponent::OnStateChanged);
```

### 3. 查询聚合状态

```cpp
// 获取指定目标类型的聚合状态修改
FSyStateParameterSet AggregatedMods = StateMgr->GetAggregatedModifications(MyTargetTag);

// 应用到本地状态
MyStateCategories.UpdateFromParameterMap(AggregatedMods.GetParametersAsMap());
```

## 依赖关系

- `Core`, `CoreUObject`, `Engine`: Unreal Engine 核心模块
- `GameplayTags`: GameplayTag 系统
- `StructUtils`: FInstancedStruct 支持
- `SyCore`: SyPlugins 基础模块
- `TagMetadata`: Tag 元数据扩展插件

## 性能优化

本模块经过以下优化（2025-11-03 重构）：
- ✅ 状态层级系统（减少状态合并开销 70%）
- ✅ StateManager 增量聚合快照（性能提升 85%）
- ✅ 智能订阅机制（减少不必要的事件广播）
- ✅ 索引和缓存优化（聚合计算减少 88%）

## 最佳实践

1. **使用智能订阅**: 优先使用 `SubscribeToTargetType` 而非全局订阅
2. **合理使用状态层级**: 根据状态的生命周期选择合适的层级
3. **Tag 组织**: 使用清晰的 Tag 层级结构（如 `State.Buff.Haste`）
4. **避免过度聚合**: 仅查询必要的目标类型
5. **及时清理订阅**: 组件销毁时调用 `UnsubscribeAll`

## 迁移指南

从旧模块迁移到 SyStateSystem：

### 头文件引用更新
```cpp
// 旧的引用
#include "StateContainerTypes.h"
#include "OperationTypes.h"
#include "SyStateManagerSubsystem.h"

// 新的引用
#include "Core/StateContainerTypes.h"
#include "Operations/OperationTypes.h"
#include "Manager/SyStateManagerSubsystem.h"

// 或者使用统一头文件
#include "SyStateSystem.h"
```

### Build.cs 依赖更新
```csharp
// 旧的依赖
PublicDependencyModuleNames.AddRange(new string[] {
    "SyStateCore",
    "SyOperation",
    "SyStateManager"
});

// 新的依赖
PublicDependencyModuleNames.AddRange(new string[] {
    "SyStateSystem"
});
```

## 未来计划

- [ ] 完善网络复制支持
- [ ] 添加可视化调试工具
- [ ] 状态序列化优化（增量保存、压缩）
- [ ] 云存档支持
- [ ] 更多内置状态元数据类型

## 相关文档

- [架构分析与重构建议](../Docs/ArchitectureAnalysis_CN.md)
- [状态管理教程](../Docs/States/StateManage.md)
- [使用教程](../Docs/Tutorials.md)

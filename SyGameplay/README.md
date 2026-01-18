# SyGameplay - 基础Gameplay玩法模块

## 模块概述 (Module Overview)

`SyGameplay` 是构建在 `SyEntity` 和 `SyStateSystem` 之上的高层Gameplay框架模块。它提供了具体的游戏交互系统实现，包括实体交互、生成器、玩家交互管理等核心游戏玩法功能。

## 核心设计理念 (Core Design Philosophy)

- **状态驱动交互**: 交互能力由实体状态控制（如 `State.Interact.Interactable`）
- **组件化扩展**: 基于 `ISyComponentInterface` 接口实现统一的组件管理
- **非侵入集成**: 可以轻松添加到现有Actor，无需修改继承结构
- **灵活交互类型**: 支持Basic和Flow两种交互模式，满足不同复杂度需求

## 关键职责 (Key Responsibilities)

- **实体交互系统**: 提供状态驱动的交互组件
- **玩家交互管理**: 管理玩家可交互目标的选择和执行
- **实体生成器**: 提供状态驱动的生成组件
- **交互元数据定义**: 定义交互相关的状态元数据类型
- **UI交互支持**: 提供交互UI管理组件

## 核心组件与概念 (Core Components & Concepts)

### 1. 交互系统 (Interaction System)

#### USyInteractionComponent
实体交互组件，为Actor提供可交互能力。

**主要职责**:
- 继承自 `UInteractionComponent`，提供基础交互功能
- 监听实体状态变化，根据 `State.Interact.Interactable` 控制交互可用性
- 读取 `State.Interact.InteractInfo` 元数据，执行交互逻辑
- 支持两种交互类型：
  - **Basic**: 触发蓝图事件，由蓝图处理交互逻辑
  - **Flow**: 自动处理FlowComponent并启动Flow资产

**关键特性**:
- 状态驱动的交互启用/禁用
- 自动处理Flow类型交互
- 为Basic类型交互提供蓝图事件
- 实现 `ISyComponentInterface` 接口

#### USyPlayerInteractionManagerComponent
玩家交互管理组件，管理玩家可交互目标的选择和执行。

**主要职责**:
- 维护当前检测范围内的可交互组件列表
- 自动选择激活的交互目标（通常是最近的）
- 提供交互执行接口
- 广播激活交互变化事件

**注意事项**:
> ⚠️ 当前使用静态事件订阅 (`UInteractionComponent::OnPlayerEnter/Exit`)，未来应改为使用专用检测组件。

#### USyInteractionUIManagerComponent
交互UI管理组件，处理交互相关的UI显示。

**主要职责**:
- 监听激活交互变化
- 管理交互提示UI的显示/隐藏
- 提供UI更新接口

### 2. 生成系统 (Spawn System)

#### USySpawnComponent
实体生成组件，为Actor提供生成器功能。

**主要职责**:
- 继承自 `USpawnComponent`，提供基础生成功能
- 监听实体状态变化，根据 `State.Spawner.Enable` 控制生成能力
- 实现 `ISyComponentInterface` 接口

### 3. 元数据定义 (Metadata Definitions)

#### 交互元数据 (`SyGameplayInteractMetadatas.h`)
定义交互相关的状态元数据类型：

- **USyInteractableMetadata**: 布尔型元数据，表示是否可交互
- **USyInteractionListMetadata**: 列表型元数据，存储交互信息列表

#### 交互值类型 (`SyGameplayInteractValueTypes.h`)
定义交互信息的结构体：

- **FSyInteractInfoBase**: 交互信息基类
- **FSyBasicInteractInfo**: Basic类型交互信息
- **FSyFlowInteractInfo**: Flow类型交互信息
- **FSyInteractionListValue**: 交互列表值类型

## 目录结构

```tree
SyGameplay/
├── Source/
│   ├── SyGameplay/
│   │   ├── Private/
│   │   │   ├── Components/
│   │   │   │   ├── Base/              (基础组件实现)
│   │   │   │   ├── SyInteractionComponent.cpp
│   │   │   │   └── SySpawnComponent.cpp
│   │   │   ├── PlayerComponents/
│   │   │   │   ├── Manager/           (玩家交互管理)
│   │   │   │   └── UI/                (交互UI管理)
│   │   │   └── Metadatas/             (元数据实现)
│   │   └── Public/
│   │       ├── SyGameplay.h
│   │       ├── Components/
│   │       │   ├── Base/              (基础组件接口)
│   │       │   ├── SyInteractionComponent.h
│   │       │   └── SySpawnComponent.h
│   │       ├── PlayerComponents/
│   │       │   ├── Manager/
│   │       │   │   └── SyPlayerInteractionManagerComponent.h
│   │       │   └── UI/
│   │       │       └── SyInteractionUIManagerComponent.h
│   │       └── Metadatas/
│   │           ├── SyGameplayInteractMetadatas.h
│   │           └── SyGameplayInteractValueTypes.h
│   └── SyGameplay.Build.cs
└── Content/                           (示例资源)
```

## 使用指南 (Usage Guide)

### 1. 设置可交互实体

```cpp
// 1. 为Actor添加必要组件
USyEntityComponent* EntityComp = Actor->AddComponent<USyEntityComponent>();
USyInteractionComponent* InteractComp = Actor->AddComponent<USyInteractionComponent>();

// 2. 通过StateManager设置交互状态（或在蓝图中配置默认状态）
FSyOperation Operation;
Operation.Source = FSyOperationSource(SourceTag);

// 设置可交互
FSyStateParameterSet Modifications;
Modifications.AddStateParam(
    FGameplayTag::RequestGameplayTag("State.Interact.Interactable"),
    TrueValue
);

// 设置交互信息
FSyFlowInteractInfo FlowInfo;
FlowInfo.FlowAsset = MyFlowAsset;
Modifications.AddStateParam(
    FGameplayTag::RequestGameplayTag("State.Interact.InteractInfo"),
    FlowInfo
);

Operation.Modifier = FSyOperationModifier(Modifications);
Operation.Target = FSyOperationTarget(TargetTypeTag);

StateManager->RecordOperation(Operation);
```

### 2. 设置玩家交互管理

```cpp
// 为玩家Controller或Pawn添加交互管理组件
USyPlayerInteractionManagerComponent* InteractMgr = 
    PlayerPawn->AddComponent<USyPlayerInteractionManagerComponent>();

// 监听激活交互变化
InteractMgr->OnActiveInteractionChanged.AddDynamic(
    this, &AMyPlayerController::OnActiveInteractionChanged
);

// 执行交互
bool bSuccess = InteractMgr->TryExecuteActiveInteraction();
```

### 3. 处理Basic类型交互

```cpp
// 在蓝图中绑定SyInteractionComponent的OnBasicInteractionRequested事件
void AMyActor::HandleBasicInteraction(USyInteractionComponent* InteractionComp)
{
    // 执行自定义交互逻辑
    UE_LOG(LogTemp, Log, TEXT("Basic interaction triggered!"));
}
```

## 依赖关系 (Dependencies)

- `SyCore` - 基础设施和组件接口
- `SyEntity` - 实体框架
- `SyStateSystem` - 状态管理系统
- `Flow` - Flow图形化流程系统
- `GameplayTags` - Gameplay标签系统
- `StructUtils` - 实例化结构体支持
- `UMG` - UI系统
- `AIModule` - AI模块（用于感知系统）
- `StateTreeModule` - 状态树模块

## 使用建议与最佳实践 (Usage Notes/Guidelines)

1. **交互状态管理**
   - 使用StateManager统一管理交互状态
   - 通过状态变更启用/禁用交互
   - 利用状态层级系统实现临时交互状态

2. **交互类型选择**
   - Basic类型适合简单触发（如拾取物品、开关门）
   - Flow类型适合复杂流程（如对话、任务交互）

3. **玩家交互管理**
   - 为每个玩家Controller/Pawn添加一个InteractionManager
   - 监听ActiveInteractionChanged事件更新UI
   - 使用TryExecuteActiveInteraction执行交互

4. **性能优化**
   - 合理设置交互检测范围
   - 使用状态驱动减少Tick开销
   - 及时清理不再需要的交互组件

5. **扩展交互类型**
   - 继承FSyInteractInfoBase创建自定义交互类型
   - 扩展USyInteractionComponent处理新的交互逻辑
   - 使用FInstancedStruct支持灵活的交互配置

## 未来计划 (Future Considerations)

- [ ] 实现专用的交互检测组件（替代静态事件订阅）
- [ ] 添加更多交互类型（StateTree、行为树等）
- [ ] 完善交互优先级系统
- [ ] 支持多目标交互
- [ ] 网络复制支持
- [ ] 交互条件系统（需要特定物品/技能等）
- [ ] 交互动画集成
- [ ] 更丰富的交互UI反馈

## 相关文档

- [交互系统文档](../Docs/Interaction/Interaction.md)
- [使用教程](../Docs/Tutorials.md)
- [主项目README](../README.md)

# SyFlowImpl Plugin

## 概述
SyFlowImpl是一个Unreal Engine插件，用于提供SyPlugins各个功能模块的Flow节点实现。该插件旨在通过Flow图形化节点的方式，使得SyPlugins的各项功能更易于组合和使用。

## 依赖项
- Flow Plugin
- SyCore Plugin
- SyStateSystem Plugin
- SyGameplay Plugin
- GameplayTags
- StructUtils

## 功能模块

### 1. 消息系统 (Messaging System)
实现与SyCore消息总线系统的集成，提供消息监听和处理的Flow节点。

#### 核心组件
- **MessageBase节点** (`USyFlowNode_MessageBase`)
  - 提供消息处理的基础框架
  - 统一的生命周期管理
  - 可扩展的过滤系统

#### 可用节点
- **ListenByType节点**
  - 功能：按消息类型监听
  - 输出：源类型、源ID、消息内容
  - 使用场景：特定类型消息监听

- **ListenBySource节点**
  - 功能：按消息源监听
  - 输出：消息类型、消息内容
  - 使用场景：特定来源消息监听

- **ListenMessage节点**
  - 功能：综合条件监听
  - 输出：完整消息内容
  - 使用场景：精确消息匹配

### 2. 状态操作系统 (State Operations)
实现与SyStateSystem状态管理的集成，提供状态操作记录的Flow节点。

#### 核心组件
- **StateOperationBase节点** (`USyFlowNode_StateOperationBase`)
  - 提供状态操作的基础框架
  - 统一的操作记录和卸载机制
  - 可扩展的Source和Modifier系统

#### 可用节点
- **ApplyStateOperation_Simple节点**
  - 功能：应用简单的状态操作
  - 配置：目标类型、修改参数
  - 使用场景：通用状态变更

- **Setup Interaction节点** (`USyFlowNode_SetupInteraction`)
  - 功能：设置实体交互状态
  - 配置：启用交互、设置交互信息
  - 使用场景：配置可交互对象

### 3. 交互系统 (Interaction System)
实现与SyGameplay交互组件的集成，提供交互控制的Flow节点。

#### 可用节点
- **Setup Interaction节点**
  - 功能：设置实体的交互状态和信息
  - 配置：交互内容（支持Flow类型和Basic类型）
  - 使用场景：启用NPC交互、触发点交互等

- **Send Interaction End Event节点** (`USyFlowNode_SendInteractionEndEvent`)
  - 功能：发送交互结束事件
  - 使用场景：标记交互流程完成

## 使用指南

### 通用设置
1. 确保依赖插件已启用
2. 在项目插件设置中启用SyFlowImpl
3. 检查相关配置文件

### 模块使用说明

#### 消息系统使用
1. **节点创建**
   - 在Flow图中选择 "SyCore|MessageBus" 分类
   - 根据需求选择合适的监听节点

2. **基础配置**
   - 设置过滤条件
   - 连接控制流
   - 处理输出事件

3. **最佳实践**
   - 选择合适的节点类型
   - 及时清理无用监听
   - 保持处理逻辑简洁

#### 状态操作使用
1. **节点创建**
   - 在Flow图中选择 "SyPlugin|StateOp" 分类
   - 选择合适的操作节点

2. **基础配置**
   - 配置操作目标（TargetTypeTag）
   - 设置状态修改参数
   - 连接In/Out/Unload流程

3. **最佳实践**
   - 使用Setup Interaction节点快速配置交互
   - 使用Unload pin撤销状态操作
   - 保持操作原子性

#### 交互系统使用
1. **节点创建**
   - 在Flow图中选择 "SyPlugin|Interaction" 分类
   - 配置交互信息结构

2. **基础配置**
   - 选择交互类型（Flow或Basic）
   - 配置交互内容
   - 连接控制流

3. **最佳实践**
   - Flow类型用于复杂对话和任务
   - Basic类型用于简单触发
   - 及时发送交互结束事件

## 开发指南

### 添加新功能模块
1. 在Source目录下创建对应模块文件夹
2. 遵循命名规范创建节点类
3. 实现必要的接口和功能
4. 更新本文档对应章节

### 代码规范
1. 使用统一的命名前缀
2. 保持节点功能单一
3. 提供完整的节点描述
4. 添加必要的注释

## 性能考虑
- 控制单图表中的节点数量
- 合理使用过滤器
- 注意资源的及时释放
- 避免复杂的节点链

## 版本信息
- 当前版本：1.0
- 作者：Syan

## 更新日志
### v1.1 (2025-11)
- 新增状态操作系统节点
- 新增交互系统节点
- 实现Setup Interaction节点
- 实现Send Interaction End Event节点

### v1.0
- 实现消息系统基础功能
- 提供三种消息监听节点
- 建立基础框架结构

## 路线图
- [ ] 添加更多状态操作快捷节点
- [ ] 实现实体查询和控制节点
- [ ] 添加更多消息处理功能
- [ ] 完善状态操作的撤销机制

## 贡献指南
1. 遵循模块化结构
2. 保持向后兼容性
3. 更新相关文档
4. 添加必要的测试用例

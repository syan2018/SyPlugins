# SyPlugins - Unreal Engine 模块化事件与状态管理系统

README in Other Language: 

<p align="left">
  <a href="./README_EN.md"><img alt="README in English" src="https://img.shields.io/badge/English-d9d9d9"></a>
</p>

## 项目概述
SyPlugins 是一个为 Unreal Engine 开发的模块化插件系统，专注于构建一个以状态驱动为核心思想的游戏框架。系统辅以消息驱动和模块化设计，实现灵活且易扩展的游戏实体基础功能。

初版使用引导见： [Tutorials](Docs/Tutorials.md)

### 项目依赖

- [ModularityPractice](https://github.com/Variann/ModularityPractice) 项目运转部分依赖 Variann 的 ModularityPractice，感谢 TagMetadata 为本项目的信息和状态管理提供核心思路
   - **TagMetadata**：将Tag与任意类型元数据关联绑定的插件
   - **FlowExtension**：简单实现、易于拓展的任务/对话系统，本项目中 SyPluginsImpl 提供的示例很大程度上拷贝于 ModularityPractice 对 FlowExtension 的原始实现
   - **TagFacts**：基于Tag和Int储存全局变量的插件，项目预期基于 TagFacts 扩充任务系统实现，支持游戏流程状态信息储存
   - 项目中其它轮子也相当值得学习！强烈推荐
- [FlowGraph](https://github.com/MothCocoon/FlowGraph) 插件系统运转的编辑实现依赖，非常易用的 Unreal Gameplay 流程管理框架

## 核心理念
- **状态驱动**: 基于"操作记录"统一管理游戏对象全局状态及其转换
- **消息驱动**: 使用消息总线实现交互和玩家行为感知（并通过Flow或其它插件进行流程管理）
- **组件化**: 使用组件标识Actor实体，**非侵入引入任何项目**；支持拓展实体组件，快捷向消息和状态系统接入既有逻辑

## 系统架构

系统简要实现下述逻辑链条

![Overview](Docs/assets/Overview.png)

## 使用参考

[Tutorials](Docs/Tutorials.md)

## 插件层级

1. **SyCore**
   - 系统的核心基础设施模块
   - **实体系统 (Entity System)**：整合了原 SyEntity 模块，提供基于组件的实体管理框架。
   - **状态系统 (State System)**：整合了原 SyStateCore, SyOperation, SyStateManager 模块，提供统一的状态管理、操作记录和分发机制。
   - **消息总线 (Message Bus)**：实现模块间解耦通信。
   - **基础服务 (Foundation)**：提供标识符、错误处理等底层工具。

2. **SyGameplay**
   - 基础 Gameplay 玩法框架
   - 构建在 SyCore 之上的具体游戏交互系统
   - 提供实体交互、玩家交互管理、生成系统等高层功能
   - 实现触发器和通用Gameplay逻辑

3. **SyFlowImpl**
   - Flow 插件实现模块
   - 提供 SyPlugins 功能的图形化节点
   - 集成消息和状态系统到 Flow Graph
   - 使 Gameplay 逻辑更易于可视化编排

4. **SyQuest**（计划中）
   - 任务系统实现（迭代当前 FlowExtension 实现）
   - 基于状态、触发和监听实现统一任务更新
   - 提供任务逻辑和状态管理

5. **SyCombat** (计划中)
   - ARPG 战斗管线模块 (SCPA 架构)
   - 基于 SyCore Processing 层实现通用的战斗数值处理
   - 提供输入缓冲、技能判定、结算链等核心战斗机制
   - 目标是建立一套即插即用的标准化战斗操作系统

6. **SyPluginsImpl**
   - 插件实现示例模块
   - 对示例使用的 GameplayTag 结构等进行汇总
   - 提供具体功能实现参考
   - 展示模块间的集成方式

各模块详细介绍详见模块下 README.md

## 项目由来

（碎碎念）

项目思路源自在前司为单机设计，但因空降乐子未能妥善落实的框架。最初的核心思路为 **分层逻辑管理** 与 **消息总线**。

1. 在游戏层面上，为流程逻辑和对象逻辑进行层级拆分
2. 层级之间构建清晰的消息传递链路，基于实体身份标识进行通信

而在近一年为在线游戏设计任务系统，积累了失败的工作经验，进一步引入了 **实体与组件化**、**状态管理** 以及一定程度上的前后端分离设计支持。

1. 基于实体标识游戏对象
2. 基于逻辑组件，为不同状态实现逻辑呈现
3. 对实体状态的全局管理追求有源、生命周期、冲突管理等
4. （待实现）逻辑的前后端分层与维护

汇总后对游戏任务和关卡底层的设计思路逐渐清晰，恰逢朋友做demo投米，遂协助搭一轮子添把柴。再加之近期辞职搞 AI 创业，搭着 Cursor 把这玩意儿写出来，拾一下 Vibe Coding 工程习惯，顺便给 TD 生涯留条退路，岂不美哉。

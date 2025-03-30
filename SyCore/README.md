# SyCore - 核心功能模块

## 模块概述
SyCore 是 SyPlugins 的基础模块，提供了实体标识、消息传递等核心功能。该模块作为其他高层模块的基础设施，确保系统的可靠性和可扩展性。

## 核心组件

### MessageBus（消息总线）
- 实现模块间通信
- 支持消息订阅和发布
- 提供消息优先级机制

### Identifier（标识系统）
- 生成唯一标识符
- 管理对象引用
- 提供快速查找机制


## 目录结构

```tree
SyCore/
├── Source/
│   ├── SyCore/
│   │   ├── Private/
│   │   │   ├── Foundation/  (基础工具)
│   │   │   ├── Identity/    (ID和引用管理)
│   │   │   ├── Messaging/   (消息与事件系统)
│   │   │   └── SyCore.cpp
│   │   └── Public/
│   │       ├── SyCore.h
│   │       ├── Foundation/
│   │       │   └── Utilities/
│   │       ├── Identity/
│   │       │   └── ···
│   │       └── Messaging/
│   │           └── ···
│   └── SyCore.Build.cs
├── SyCore.uplugin
└── README.md
```


## 使用指南

### Identity 身份标识模块

由于 ActorComponent 无法处理 Construction Script，因此 SyIdentityComponent 无法在编辑器中自动初始化


相关模块理想的使用流程如下
1. 项目根据自身使用需求，构建通用Actor模板，后续所有含逻辑Actor继承自该模板
2. 在Actor模板中添加SyEntityComponent组件，随后自动添加Identity、Messaging等组件
3. 在Actor的Construction Script中调用SyEntity的初始化逻辑，调用到Identity的初始化，相关初始化逻辑继承
4. 设计师为具体BPActor配置默认的GameplayTag，确定作用域（NPC、敌人、关卡元件）
5. 设计师拖拽BP进入场景，BP自动生成UUID，并完成Identifier的初始化，支持后续消息组件能基于Identity组件与消息总线通信


### MessageBus 消息总线

包含消息组件和用于转发消息的总线，消息发送依赖于消息组件和Identity组件初始化完成

消息包含两个部分
- 消息来源：来源身份GameplayTag + 标识（UID和别名）
- 消息内容：消息类型GameplayTag + 内容信息（允许基于TagMetadata插件实现对Tag对应内容的标记，TODO实现）

总线的输出主要流向 任务、蓝图、事实 等监听性质的系统，自身通常不需要处理状态管理，主要整转发；后续各自模块的状态记录各自实现

在模块下可以实现部分几个Flow的监听节点：
- 基于来源监听，输出消息
- 基于消息监听，输出来源
- 基于来源、消息的组合监听
回头检查下如何实现2.0更新后带数值端口的节点（）


## 最佳实践
1. 使用接口而非直接引用
2. 优先使用消息通信
3. 保持状态简单且原子
4. 正确管理组件生命周期
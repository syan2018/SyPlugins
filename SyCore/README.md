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
│   │       │   ├── SyIdentifier.h
│   │       │   └── ···
│   │       └── Messaging/
│   │           ├── SyMessageBus.h
│   │           └── ···
│   └── SyCore.Build.cs
├── SyCore.uplugin
└── README.md
```


## 使用指南


## 最佳实践
1. 使用接口而非直接引用
2. 优先使用消息通信
3. 保持状态简单且原子
4. 正确管理组件生命周期
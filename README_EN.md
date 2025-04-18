# SyPlugins - Unreal Engine Modular Event and State Management System

Translated by Gemini 2.5 Pro Exp

## Project Overview
SyPlugins is a modular plugin system developed for Unreal Engine, focusing on building a game framework centered around the core idea of being state-driven. The system is complemented by message-driven and modular design principles to achieve flexible and easily extensible fundamental functionalities for game entities.


### Project Dependencies

- [ModularityPractice](https://github.com/Variann/ModularityPractice): Parts of the project's operation rely on Variann's ModularityPractice. Thanks to TagMetadata for providing the core ideas for information and state management in this project.
   - **TagMetadata**: A plugin that binds Tags with metadata of any type.
   - **FlowExtension**: A simple and easily extensible quest/dialogue system. The examples provided in SyPluginsImpl in this project are largely copied from ModularityPractice's original implementation of FlowExtension.
   - **TagFacts**: A plugin for storing global variables based on Tags and Integers. The project plans to expand the quest system based on TagFacts, supporting the storage of game flow state information.
   - Other tools in the project are also worth learning! Highly recommended.
- [FlowGraph](https://github.com/MothCocoon/FlowGraph): The system relies on this for its editor implementation; a very user-friendly Unreal Gameplay flow management framework.




## Core Concepts
- **State-Driven**: Uniformly manage the global state and transitions of game objects based on "operation records".
- **Message-Driven**: Use a message bus to implement interaction and perceive player behavior (and manage processes through Flow or other plugins).
- **Componentization**: Identify Actor entities using components, allowing **non-intrusive integration into any project**; supports extending entity components to quickly connect existing logic to the message and state systems.

## System Architecture

The system briefly implements the following logic chain:

![Overview](Docs/Images/Overview.png)


## Plugin Hierarchy
1.  **SyCore**
    - The infrastructure module of the system.
    - Provides a unique identifier system for entities (Identifier).
    - Implements a message bus for inter-module communication (MessageBus).
    - Defines some core data structures.
    - Serves as the base dependency for all other SyPlugins modules.

2.  **SyStateCore**
    - The core definition module for the state system.
    - Defines the data structure for entity states (`FSyEntityState`).
    - Provides state initialization configuration (`FSyEntityInitData`).
    - Defines various state metadata classes (extending `UO_TagMetadata`).
    - Provides interfaces for accessing state data.

3.  **SyStateManager**
    - The centralized module for state management.
    - Provides mechanisms for recording and distributing state changes.
    - Implements the authoritative source for state synchronization.
    - Manages state modification records (`FSyStateModificationRecord`).
    - Provides state querying and event notification.

4.  **SyOperation**
    - The state operation definition module.
    - Defines data structures for state change operations.
    - Provides a parameter schema system.
    - Implements common parameter structures.
    - Supports editor configuration and validation.

5.  **SyEntity**
    - The generic entity framework module.
    - Provides a component-based entity management framework.
    - Implements entity state management (via `SyStateComponent`).
    - Supports non-intrusive Actor extension.
    - Provides entity registration and querying functions.

6.  **SyFlowImpl**
    - The Flow plugin implementation module.
    - Provides graphical nodes for SyPlugins functionalities.
    - Implements Flow node integration for the message system.
    - Supports Flow nodes for entity control (planned).
    - Makes SyPlugins functionalities easier to compose and use.

7.  **SyPluginsImpl**
    - The plugin implementation example module.
    - Consolidates structures like `GameplayTag` used in examples.
    - Provides references for specific function implementations.
    - Demonstrates integration methods between modules.

8.  **SyGameplay**
    - The basic Gameplay framework module.
    - Builds specific game interaction systems on top of `SyEntity`.
    - Provides higher-level functionalities like entity interaction.
    - Currently implements simple triggers, generic interactions, etc.

9.  **SyQuest** (Planned)
    - Quest system implementation (iterating on the current FlowExtension implementation).
    - Achieves unified quest updates based on states, triggers, and listeners.
    - Provides quest logic and state management.
    - Includes importing and refactoring the TagFacts system.

Detailed introductions for each module can be found in their respective README.md files. Please forgive if the project structure seems overly granular.


## TODO

(To be organized)


## Project Origin

(Ramblings)



The project idea originated from a framework designed for a single-player game at my previous company, which couldn't be properly implemented due to some unforeseen circumstances [[记录]设计一套通用的游戏事件底层 – 再见，避风港！](http://blog.cyasylum.top/index.php/2024/03/31/%e8%ae%b0%e5%bd%95%e8%ae%be%e8%ae%a1%e4%b8%80%e5%a5%97%e9%80%9a%e7%94%a8%e7%9a%84%e6%b8%b8%e6%88%8f%e4%ba%8b%e4%bb%b6%e5%ba%95%e5%b1%82/) (Link in Chinese). The initial core ideas were **layered logic management** and a **message bus**.

1.  At the game level, separate flow logic and object logic into layers.
2.  Establish clear message passing links between layers, communicating based on entity identifiers.

Over the past year, while designing a quest system for an online game, I accumulated some unsuccessful work experience, which led to the introduction of **entities and componentization**, **state management**, and support for a certain degree of front-end/back-end separation design.

1.  Identify game objects based on entity identifiers.
2.  Implement logic presentation for different states based on logical components.
3.  Pursue source tracking, lifecycle management, conflict management, etc., for global entity state management.
4.  (To be implemented) Layering and maintenance of front-end and back-end logic.

After consolidating these ideas, the design approach for the underlying game quest and level systems became clearer. Coincidentally, a friend was making a demo for miHoYo, so I helped build a tool to contribute. Additionally, having recently resigned to start an AI venture, I used Cursor to write this out, reinforcing Vibe Coding engineering habits and, incidentally, leaving a fallback option for my TD career. Isn't that wonderful? 
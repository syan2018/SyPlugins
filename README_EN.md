# SyPlugins - Unreal Engine Modular Event and State Management System

> Translated by Gemini 2.5 Pro Exp

## Project Overview
SyPlugins is a modular plugin system developed for Unreal Engine, focusing on building a game framework centered around the core idea of being state-driven. The system is complemented by message-driven and modular design principles to achieve flexible and easily extensible fundamental functionalities for game entities.

Tutorials in: [Tutorials](Docs/Tutorials_EN.md)

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

![Overview](Docs/assets/Overview.png)


## Plugin Hierarchy

1. **SyCore**
   - The infrastructure module of the system
   - Provides a unique identifier system for entities (Identifier)
   - Implements a message bus for inter-module communication (MessageBus)
   - Defines core data structures
   - Serves as the base dependency for all other SyPlugins modules

2. **SyStateSystem** (Unified State Management System, integrating former SyStateCore, SyOperation, SyStateManager)
   - **Core Subsystem**: Core data definitions for the state system
     - Defines entity state data structures (FSyStateCategories)
     - Provides editor configuration support for states (FSyStateParameterSet)
     - Defines various state metadata classes (extending UO_TagMetadata)
     - Supports layered state management (FSyLayeredStateContainer)
   - **Operations Subsystem**: State operation definitions
     - Defines state change operation data structures (FSyOperation)
     - Provides parameter Schema system
     - Implements common parameter structures
     - Supports editor configuration and validation
   - **Manager Subsystem**: State management center
     - Provides state change recording and distribution mechanisms
     - Implements the authoritative source for state synchronization
     - Manages state modification records (FSyStateModificationRecord)
     - Provides state querying and event notification
     - Smart subscription and performance optimization

3. **SyEntity**
   - Generic entity framework module
   - Provides component-based entity management framework
   - Implements entity state management (via SyStateComponent)
   - Supports non-intrusive Actor extension
   - Provides entity registration and querying functions

4. **SyFlowImpl**
   - Flow plugin implementation module
   - Provides graphical nodes for SyPlugins functionalities
   - Implements Flow node integration for the message system
   - Provides state operation nodes
   - Provides interaction system nodes
   - Makes SyPlugins functionalities easier to compose and use

5. **SyGameplay**
   - Basic Gameplay framework module
   - Builds on top of SyEntity for specific game interaction systems
   - Provides entity interaction and other high-level functionalities
   - Currently implements triggers, generic interactions, spawn system, etc.

6. **SyQuest** (Planned)
   - Quest system implementation (iterating on current FlowExtension implementation)
   - Achieves unified quest updates based on states, triggers, and listeners
   - Provides quest logic and state management
   - Includes importing and refactoring the TagFacts system

7. **SyPluginsImpl**
   - Plugin implementation example module
   - Consolidates structures like GameplayTag used in examples
   - Provides references for specific function implementations
   - Demonstrates integration methods between modules

Detailed introductions for each module can be found in their respective README.md files. Please forgive if the project structure seems overly granular.

## Usage Reference

[Tutorials](Docs/Tutorials_EN.md)

## Project Origin

(Personal Reflections)

The project's ideas originated from a framework designed for single-player games at my previous company, which, due to some unexpected changes, was not properly implemented. The initial core concepts were **layered logic management** and **message bus**.

1. At the game level, separate process logic and object logic into hierarchical layers
2. Build clear message passing chains between layers, communicating based on entity identity

Over the past year, while designing quest systems for online games and accumulating (failed) work experience, I further introduced **entity and componentization**, **state management**, and some degree of frontend-backend separation design support.

1. Identify game objects based on entity identifiers
2. Implement logic presentation for different states based on logic components
3. Pursue centralized management of entity states with source tracking, lifecycle management, conflict resolution, etc.
4. (To be implemented) Frontend-backend layering and maintenance of logic

After consolidating these ideas, the design approach for game quests and level foundations gradually became clear. As a friend was working on a demo for pitching, I assisted in building this framework. Combined with my recent resignation to pursue AI entrepreneurship, I wrote this thing with Cursor, practicing Vibe Coding engineering habits while leaving a fallback for my TD career. Isn't that wonderful?

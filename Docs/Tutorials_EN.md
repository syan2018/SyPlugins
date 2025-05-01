# Usage Guide

> Translated by Gemini 2.5 Pro Exp

By following the steps below, you can quickly apply SyPlugins in your project.

The process mainly includes several parts:

1.  **Importing SyEntity related basic Features**
    1.  Associated Plugins: `SyCore`, `SyEntity`
2.  **Using Flow and SyState for State Management**
    1.  Associated Plugins: `SyStateCore`, `SyStateManager`, `SyFlowImpl`, `SyOperation`
    2.  Dependencies: `TagMetadata`, `Flow`
3.  **Testing Task Execution with FlowExtension**
    1.  Associated Plugins: `SyGameplay`, `SyPluginsImpl`
    2.  Dependencies: `FlowExtension`

## I. Importing SyEntity Related Basic Features

By adding the `SyEntityComponent` to any Actor, we can efficiently identify it as an Actor entity within the `SyEntity` system, allowing the use of the universal state management and interaction interfaces provided by SyPlugins.

![](https://w7bm0b0qko.feishu.cn/space/api/box/stream/download/asynccode/?code=NGFiN2I3YWQzZWEwYzcwYjFhN2EwOGE0YWZmNzc4YTdfUmpUVG1ZcFhnUTNpSmVTaXJKUVVnVHJkakNqdW9tVUFfVG9rZW46UUhBNmI0ZlNNb1lva1V4NkpuWGN3NjRPbm9kXzE3NDYxMTIxOTg6MTc0NjExNTc5OF9WNA)

After adding the component, `SyEntity` automatically associates a `SyIdentity` panel for managing the object's unique identifier and index.

Typically, you'll want to update the Actor's **Construction Script**, calling the `Initialize Entity` method of the `SyEntity` component within it. This ensures that the relevant Actor automatically creates its unique ID when placed in the scene.

![](https://w7bm0b0qko.feishu.cn/space/api/box/stream/download/asynccode/?code=OGFlMjU1ZTA1NGU5YmJjNjYwNWQxMjNjMzQwNDhhZGZfZUdDeWVkUjBwRGRnS1VQV1dEaWNyVUVxZGoxVXN0WlZfVG9rZW46WGE1cGI1dWh3b3E5Tkh4a3VRVWNIQ1dvbmNiXzE3NDYxMTIxOTg6MTc0NjExNTc5OF9WNA)

Simultaneously, you can configure default values for `Entity Tags` and `Entity Alias` in the **World Outliner**. Common usage involves using `EntityTags` to mark categories and `EntityAlias` to identify specific objects.

For example, to identify a trigger trap:

*   `EntityTags`: `Entity.Trigger`
*   `EntityAlias`: `Trap` (general alias) / `Trap_1-2` (process-specific object alias)

(Consider extending alias matching rules later to support prefix recognition.)

![](https://w7bm0b0qko.feishu.cn/space/api/box/stream/download/asynccode/?code=MjUxZDRlYjBjMWY5OGVjZmM0ZGRkNzkwZjgyNzRmZDRfbkQ0QXdmMmRVS1lDaU5mUnpMNTdxZWFESDFwZ2hzOXZfVG9rZW46REx4NGJobko3b2p4MVp4dUlRNWN3MHhMblljXzE3NDYxMTIxOTg6MTc0NjExNTc5OF9WNA)

Overall, after dragging the entity into the scene, the related object is initialized. `SyEntity` also automatically creates associations for several basic functional components:

*   `SyIdentity`: The identification function is split into a separate component.
*   `SyMessage`: Provides unified message transmission functionality.
*   `SyState`: Manages the entity's state, introduced in the second section.
*   Other components: Need to be added manually, introduced in the third section.

Through the basic functions of these components, `SyEntity` can organically interact with various elements in the game.

## II. Using Flow and SyState for State Management

Before officially starting `SyState` related management, we need to configure `Tag` and `TagMetadata`.

### Introduction to TagMetadata

`TagMetadata` is a convenient tool provided in [Variann](https://github.com/Variann) / [ModularityPractice](https://github.com/Variann/ModularityPractice) used to bind `Tags` with the data structures they can carry. It's a very flexible and useful tool.

You can support it on the Fab marketplace: [Tags Metadata | Fab](https://www.fab.com/listings/fb72edfb-c377-4116-b4e4-f4486479bcfa)

![](https://w7bm0b0qko.feishu.cn/space/api/box/stream/download/asynccode/?code=ZjhmZDc3MzU5ZDUzOTkyMzUzMDlkYTJkNGQ1NDNkZjJfTVN6eG9Cdmt5MDNTaHZycnRKbEZBM1cxOG5rdUtoc05fVG9rZW46UzE4bmJJZXVkb04zZUp4OXozZWNxWVQxbmZiXzE3NDYxMTIxOTg6MTc0NjExNTc5OF9WNA)

In SyPlugins, we use this feature to manage entity states, supporting the attachment of state parameters, interaction assets, etc., as states to entities.

### Configuration of Tags and TagMetadata

The project needs `Tag` and `TagMetadata` configured for the basic setup to be available.

Go to **Project Settings -> Project -> GameplayTags** to add preset `Tag` configurations. These presets are stored in a `DataTable` with the `GameplayTagTableRow` row type.

![](https://w7bm0b0qko.feishu.cn/space/api/box/stream/download/asynccode/?code=OTVhNTBhNDNhYmRhNTIwYzE0MGMzNmRhM2QyZDg5MmVfZjlyMXk2TE1KUVRRNk9SM1VyaGd1UEhzRDlmdjhwVDdfVG9rZW46SmVVcWI4ODBRb0pWMzN4UXZuU2M4QlJmbkhlXzE3NDYxMTIxOTg6MTc0NjExNTc5OF9WNA)

Here, we manage the `Tags` used within a single plugin through composite data tables for quick importing.

Similarly, we need to configure the `TagMetadata` asset elsewhere, located in **Project Settings -> Plugins -> TagMetadata Settings**.

![](https://w7bm0b0qko.feishu.cn/space/api/box/stream/download/asynccode/?code=OGExNzM1MjJhZjVlZTg3MjU4MGU0MmFlMjk1ZTBmODBfTHNOeE9VTHd1MmFwQ2E2ME5TcWd1V1RpT2l3eWlrRzlfVG9rZW46TG1IU2JlREk4b3gzaVB4djZaTmN0a0pHbjlYXzE3NDYxMTIxOTg6MTc0NjExNTc5OF9WNA)

By inheriting from `OTagMetadataCollection` and creating a Data Asset TMDC (Tag Metadata Data Collection), we can manage the `Tags` used in the project and the data structures associated with them.

We have imported mappings for two basic `Tags` and their data structures here:

*   `State.Interact.Interactable`
    *   Purpose: Stores the entity's current interactable state.
    *   Metadata: `SyBoolMetadata`
        *   A `bool` wrapped in an `FStruct` to support `FInstancedStruct` serialization.
*   `State.Interact.InteractInfo`
    *   Purpose: Stores the list of interactions the entity can currently perform.
    *   Metadata: `SyInteractionListMetadata`
        *   Used to store multiple `SyInteractionInfo` objects.
        *   Supports storing and merging multiple interactions, but the display logic is not yet handled.

With these configurations, we can add state `Tags` to `SyEntity` and manage state data.

Note: After completing the above operations, you can select **Content Browser -> Plugins -> SyPluginsImpl Content -> Maps -> Level_SyTest** to view the example scene.

### Checking Entity State

For entities that require predefined states, we can add a new `SyState` component to manage detailed preset states.

Under **SyState -> Config -> Default Initialization Data**, we can add multiple sets of state parameters (support for quickly reading from assets will be added later to avoid repetitive configuration).

(Note: Due to an esoteric UE5.5 editor bug, please do not mistakenly edit the `SyState` dropdown menu on the `SyEntity` panel, as it's an uneditable incorrect display. If anyone knows how to make these things display better in the details panel, please teach me.)

As shown below, we define an entity's initial interactable state and its interaction logic (depends on the interaction component. Also, due to the current version not properly handling functional component initialization logic, the interaction component initialization might fail to read... update pending).

<div style="display: flex; flex-direction: row; gap: 20px;">
    <div style="flex: 1;">
        <img src="https://w7bm0b0qko.feishu.cn/space/api/box/stream/download/asynccode/?code=YTAxNTViOTZhMzZhM2ZmZDQ0NDEwYzkwYWU3ZDU5NjVfWkV1U2lNWVJETTNHY0pYQ1pjTmVUN3B1TVhUN3BWQzNfVG9rZW46TDRaU2JKaFgzb0RBZTZ4ZkV2dmNDNHVCbmRiXzE3NDYxMTIxOTg6MTc0NjExNTc5OF9WNA" alt="State Configuration" style="max-width: 100%;">
    </div>
    <div style="flex: 1;">
        <img src="https://w7bm0b0qko.feishu.cn/space/api/box/stream/download/asynccode/?code=NTcyOTU3ZmQzOTc1MGFiYjEyYTdjMTA2MzQ4ZGU2NmVfemZWOFVZR0owUUVnUTZPTDVjN2tITE5YSXJaQTNDckhfVG9rZW46RmE4VGJESmphb3FuVDl4YW5USWNnZFZybkxkXzE3NDYxMTIxOTg6MTc0NjExNTc5OF9WNA" alt="Runtime State" style="max-width: 100%;">
    </div>
</div>

During gameplay, the relevant initialization settings are automatically updated to the entity's runtime state, as shown on the right.

### Updating Entity State via Flow

Currently, SyPlugins provides a solution for setting entity states at runtime using Flow.

We can execute Flow in various ways, such as in the `GameInstance` or `World`. Here's a quick solution: register it quickly in the `BeginPlay` event of a Blueprint Actor (BPActor).

![](https://w7bm0b0qko.feishu.cn/space/api/box/stream/download/asynccode/?code=OGRkMTdkNjkyNzEwZmZkYzJmMDIyMThmMWEzMjI3NTRfcHFNN0UyNFJpdG5kYXJPWmxiWmdrdEhXUWVoMGdZeTZfVG9rZW46WlYwaGJuRXhJb1NuN2R4QjlQSWNtVlpsbm5kXzE3NDYxMTIxOTg6MTc0NjExNTc5OF9WNA)

Two sets of nodes are currently provided for quick state operations:

![](https://w7bm0b0qko.feishu.cn/space/api/box/stream/download/asynccode/?code=NWZjZjZmNDc1NmU4Mjk3YjM4N2Q2NWVmNGI5YzU1Mjlfc3RZaFRYanRnMXVhZzNyWDlWUVpPUEFZV3VxcjZqVGJfVG9rZW46R0Z5amJFNHVUb2RLaW14cWxXV2NaYUw3blVkXzE3NDYxMTIxOTg6MTc0NjExNTc5OF9WNA)

As shown, the nodes support adding specified state modifications to entities that meet the conditions.

The `SetupInteractionState` node encapsulates modifications for interactable state and interaction content, making configuration more convenient. The `ApplyStateOp` node supports quickly modifying a single state.

![](https://w7bm0b0qko.feishu.cn/space/api/box/stream/download/asynccode/?code=YzU0NzFmNzNhZDI2MjcxYTA1MTExYTJhNGIyNTg1OTZfNk55Z1Z5ZWp0YUs2WDV3YldjZTdqS0pxZTk0Wmd6a0hfVG9rZW46SlA4VmJpdFhsb0lPcmV4QTU1Y2MwYTQ4bk5GXzE3NDYxMTIxOTg6MTc0NjExNTc5OF9WNA)

As shown, after Flow executes the `SetupInteractionState` node, the specified state is attached to the character entity's Global state and merged with the Local data (overwriting if necessary) to generate the final state.

(Note: UE has another baffling bug. When calling state merging for the second time, the Local state is lost in the code, the collection becomes Empty, but the panel displays normally. This bug makes the default state configuration unstable, rescue pending.)

## III. Testing Task Execution with FlowExtension

By navigating to **Content Browser -> Plugins -> SyPluginsImpl Content -> Maps -> Level_SyTest**, you can view the example scene and the basic operation of tasks within it.

The basic tasks and display refer to the example project in [ModularityPractice](https://github.com/Variann/ModularityPractice).

By defining the `/SyPluginsImpl/Quests/QuestAssets/Q_SyTest` file, we define several simple tasks and use Flow to chain state management and update task conditions.

The Flow file can be found at `/SyPluginsImpl/Quests/FlowAssets/FA_SyPluginTest`.

![](https://w7bm0b0qko.feishu.cn/space/api/box/stream/download/asynccode/?code=YTM1MmZmMDEwMWY1NzBkODFiOWMzZDc2NzYwOWZlYTVfa2Z2TjdFZjJoUUhnenE4MVZDbU9QZUJYb0JiRlBBQVpfVG9rZW46SVhzNGJKT3kwb3dlZ1N4SmVxTWNhU3cxbnljXzE3NDYxMTIxOTg6MTc0NjExNTc5OF9WNA)

The basic logic is:

*   Track multiple states of the task: Accepted, Completed.
*   Define the logic under task states based on the flow, including listening for triggers, setting states, etc.
*   Manage trigger entry points located on entities based on state to advance the task flow.

In the example Blueprint, we simply handle two different sets of listening logic:

*   Trigger listening
*   Interaction listening

Both are sent using the `SyMessage` component, based on the identity defined by `SyIdentity`, through interfaces exposed by `SyEntity` (like Blueprints, scripts, or Flow running on the Actor).

![](https://w7bm0b0qko.feishu.cn/space/api/box/stream/download/asynccode/?code=MWQ2ZjE2ZmY1MDZmMDI0NjBhNmFlOGNjNTE0MmRjOGNfaG5jWmdWelI5UVR2aVJ1TzBTb1NUTzVVR0cwaG10OVRfVG9rZW46WFZENmJ5c29Yb3V3VDh4TG80NmNib2czblpmXzE3NDYxMTIxOTg6MTc0NjExNTc5OF9WNA)

(The image above shows a relatively crude `TriggerActor` implementation. Ideally, before sending the message, it should also check if its own state is activated, lol.)

(The image below shows an entity triggering Flow via `SyInteractComponent` and sending an interaction complete message via the `SendInteractEnd` node. The interaction start message is handled similarly in C++.)

Based on this message bus, the connection between Entity / State / Interaction is simply implemented, initially defining an easily extensible plugin framework. Feel free to discuss and use it together. 
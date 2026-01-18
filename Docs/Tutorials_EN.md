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

![](assets/network-asset-asynccode-20250502115448-g12jxqi.png)

After adding the component, `SyEntity` automatically associates a `SyIdentity` panel for managing the object's unique identifier and index.

Typically, you'll want to update the Actor's **Construction Script**, calling the `Initialize Entity` method of the `SyEntity` component within it. This ensures that the relevant Actor automatically creates its unique ID when placed in the scene.

![](assets/network-asset-asynccode-20250502115448-d7qojum.png)

Simultaneously, you can configure default values for `Entity Tags` and `Entity Alias` in the **World Outliner**. Common usage involves using `EntityTags` to mark categories and `EntityAlias` to identify specific objects.

For example, to identify a trigger trap:

*   `EntityTags`: `Entity.Trigger`
*   `EntityAlias`: `Trap` (general alias) / `Trap_1-2` (process-specific object alias)

(Consider extending alias matching rules later to support prefix recognition.)

![](assets/network-asset-asynccode-20250502115448-a0536w1.png)

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

![](assets/network-asset-asynccode-20250502115449-a58ipi8.png)

In SyPlugins, we use this feature to manage entity states, supporting the attachment of state parameters, interaction assets, etc., as states to entities.

### Configuration of Tags and TagMetadata

The project needs `Tag` and `TagMetadata` configured for the basic setup to be available.

Go to **Project Settings -> Project -> GameplayTags** to add preset `Tag` configurations. These presets are stored in a `DataTable` with the `GameplayTagTableRow` row type.

![](assets/network-asset-asynccode-20250502115449-t1jox6d.png)

Here, we manage the `Tags` used within a single plugin through composite data tables for quick importing.

Similarly, we need to configure the `TagMetadata` asset elsewhere, located in **Project Settings -> Plugins -> TagMetadata Settings**.

![](assets/network-asset-asynccode-20250502115449-v6txiaa.png)

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
        <img src="assets/network-asset-asynccode-20250502115449-tpufz9m.png" alt="State Configuration" style="max-width: 100%;">
    </div>
    <div style="flex: 1;">
        <img src="assets/network-asset-asynccode-20250502115449-lndh6c9.png" alt="Runtime State" style="max-width: 100%;">
    </div>
</div>

During gameplay, the relevant initialization settings are automatically updated to the entity's runtime state, as shown on the right.

### Updating Entity State via Flow

Currently, SyPlugins provides a solution for setting entity states at runtime using Flow.

We can execute Flow in various ways, such as in the `GameInstance` or `World`. Here's a quick solution: register it quickly in the `BeginPlay` event of a Blueprint Actor (BPActor).

![](assets/network-asset-asynccode-20250502115449-4ag170a.png)

Two sets of nodes are currently provided for quick state operations:

![](assets/network-asset-asynccode-20250502115450-e7nxtsb.png)

As shown, the nodes support adding specified state modifications to entities that meet the conditions.

The `SetupInteractionState` node encapsulates modifications for interactable state and interaction content, making configuration more convenient. The `ApplyStateOp` node supports quickly modifying a single state.

![](assets/network-asset-asynccode-20250502115450-3u8ak2j.png)

As shown, after Flow executes the `SetupInteractionState` node, the specified state is attached to the character entity's Global state and merged with the Local data (overwriting if necessary) to generate the final state.

(Note: UE has another baffling bug. When calling state merging for the second time, the Local state is lost in the code, the collection becomes Empty, but the panel displays normally. This bug makes the default state configuration unstable, rescue pending.)

## III. Testing Task Execution with FlowExtension

By navigating to **Content Browser -> Plugins -> SyPluginsImpl Content -> Maps -> Level_SyTest**, you can view the example scene and the basic operation of tasks within it.

The basic tasks and display refer to the example project in [ModularityPractice](https://github.com/Variann/ModularityPractice).

By defining the `/SyPluginsImpl/Quests/QuestAssets/Q_SyTest` file, we define several simple tasks and use Flow to chain state management and update task conditions.

The Flow file can be found at `/SyPluginsImpl/Quests/FlowAssets/FA_SyPluginTest`.

![](assets/network-asset-asynccode-20250502115450-ixh2ucf.png)

The basic logic is:

*   Track multiple states of the task: Accepted, Completed.
*   Define the logic under task states based on the flow, including listening for triggers, setting states, etc.
*   Manage trigger entry points located on entities based on state to advance the task flow.

In the example Blueprint, we simply handle two different sets of listening logic:

*   Trigger listening
*   Interaction listening

Both are sent using the `SyMessage` component, based on the identity defined by `SyIdentity`, through interfaces exposed by `SyEntity` (like Blueprints, scripts, or Flow running on the Actor).

![](assets/network-asset-asynccode-20250502115450-3uyd0k5.png)

(The image above shows a relatively crude `TriggerActor` implementation. Ideally, before sending the message, it should also check if its own state is activated, lol.)

(The image below shows an entity triggering Flow via `SyInteractComponent` and sending an interaction complete message via the `SendInteractEnd` node. The interaction start message is handled similarly in C++.)

Based on this message bus, the connection between Entity / State / Interaction is simply implemented, initially defining an easily extensible plugin framework. Feel free to discuss and use it together.

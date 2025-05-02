# 使用说明

　　通过下列流程，我们能快速在项目中应用 SyPlugins

　　流程主要包含几个部分：

1. 导入 SyEntity 相关基础 Feature

   1. 关联插件：SyCore、SyEntity
2. 使用 Flow 和 SyState 进行状态管理

   1. 关联插件：SyStateCore、SyStateManager、SyFlowImpl、SyOperation
   2. 依赖：TagMetadata、Flow
3. 结合 FlowExtension 测试任务运行

   1. 关联插件：SyGameplay、SyPluginsImpl
   2. 依赖：FlowExtension

## 一、导入 SyEntity 相关基础 Feature

　　通过为任意 Actor 添加 SyEntityComponent 组件，我们能高效将其识别为 SyEntity 体系下的 Actor 实体，进而使用 SyPlugins 中的通用状态管理和交互接口。

![](assets/network-asset-asynccode-20250502115448-g12jxqi.png)

　　在完成添加后，SyEntity 会自动关联一个 SyIdentity 面板，用于管理对象的唯一标识和索引。

　　通常，我们会希望更新 Actor 的 Construction Script，在其中调用 SyEntity 组件的 Initialize Entity 方法，使得相关 Actor 在置入场景时，自动创建其唯一 Id。

![](assets/network-asset-asynccode-20250502115448-d7qojum.png)

　　同时，大纲中可以配置 Entity Tags 和 Entity Alias 的默认值，常见用法为使用 EntityTags 标注类别，EntityAlias 标明对象。

　　如标识一个触发陷阱：

* EntityTags：Entity.Trigger
* EntityAlias：Trap（通用别名） / Trap\_1-2（流程专用对象别名）

　　（后续可以考虑拓展别名的匹配规则，支持按前缀识别）

![](assets/network-asset-asynccode-20250502115448-a0536w1.png)

　　整体而言，拖入实体后相关对象即初始化完成，同时会默认创建 SyEntity 对几个基础功能组件的关联：

* SyIdentity：即标识功能被拆分为单独的组件
* SyMessage：具有统一的消息传输功能
* SyState：具有实体的状态管理功能，在第二节中介绍
* 其它组件：需手动添加，会在第三节中介绍

　　通过这些组件的基本功能，SyEntity 能与游戏中各要素进行有机的互动

## 二、使用 Flow 和 SyState 进行状态管理

　　在正式开始 SyState 相关管理前，我们需要先进行 Tag 和 TagMetadata 的相关配置。

### TagMetadata 介绍

　　TagMetadata 是 [Variann](https://github.com/Variann) / [ModularityPractice](https://github.com/Variann/ModularityPractice) 中提供的一个便捷工具，用于将 Tag 与其可携带的数据结构绑定，是一个相当灵活好用的工具

　　可以前往 Fab 商城支持 [Tags Metadata | Fab](https://www.fab.com/listings/fb72edfb-c377-4116-b4e4-f4486479bcfa)

![](assets/network-asset-asynccode-20250502115449-a58ipi8.png)

　　在 SyPlugins 中，我们使用相关功能实现对实体状态的管理，支持为实体挂接状态参数、交互资产等作为状态。

### Tag 和 TagMetadata 的配置

　　项目需要配置 Tag 和 TagMetadata 使得基础配置可用

　　前往 **项目设置 -&gt;**  **项目 -&gt;**  **GameplayTags** 中，即可添加预设的 Tags 配置，这些预设配置通过 GameplayTagTableRow 行类型的 DataTable 储存。

![](assets/network-asset-asynccode-20250502115449-t1jox6d.png)

　　在这里我我们通过合成数据表格管理单个插件下使用到的 Tags，方便快速导入。

　　同样，我们需要前往另一处配置 TagMetadata 资产，位于 **项目设置 -&gt;**  **插件 -&gt;**  **TagMetadata Settings** 中。

![](assets/network-asset-asynccode-20250502115449-v6txiaa.png)

　　通过继承 OTagMetadataCollection，创建数据蓝图资产 TMDC，我们能对项目使用的 Tag 和与 Tag 关联的数据结构进行管理

　　我们在此导入了两个基础 Tag 和数据结构的匹配：

* State.Interact.Interactable

  * 作用：储存实体当前的可交互状态
  * Metadata：SyBoolMetadata

    * 为被封装成 FStruct 的 Bool，用于支持 FInstancedStruct 序列化
* State.Interact.InteractInfo

  * 作用：储存实体当前可执行的交互列表
  * Metadata：SyInteractionListMetadata

    * 用于储存多个 SyInteractionInfo 对象
    * 支持多个交互的储存和合并，但现实逻辑还未处理

　　通过相关配置，我们允许为 SyEntity 添加状态 Tags，并管理状态数据

　　注：完成上述操作后，即可选中 内容浏览器 -\> Plugins -\> SyPluginsImpl Content -\> Maps -\> Level\_SyTest 查看示例场景

### 实体 State 的检查

　　对于部分希望预定义状态的实体，我们可以添加一个新的 SyState 组件，用于管理细节预设状态

　　在 SyState -\> Config -\> Default Initialization Data 下，我们能添加多组状态参数（后面待支持快速从资产读取，避免重复配置）

　　（注，由于 UE5.5 玄学编辑器 bug，请勿错误编辑 SyEntity 面板上的 SyState 选单，其是无法编辑的错误显示；如果有人能捋清楚如何让这些逼玩意儿在细节面板上更好地展示，请教教我）

　　如下图所示我们定义了一个实体初始的可交互状态及其交互逻辑（依赖交互组件。另，由于当前版本未妥善梳理功能 Comp 初始化逻辑，交互组件初始化会无法读取。。。待更新）

<div style="display: flex; flex-direction: row; gap: 20px;">
    <div style="flex: 1;">
        <img src="assets/network-asset-asynccode-20250502115449-tpufz9m.png" alt="State Configuration" style="max-width: 100%;">
    </div>
    <div style="flex: 1;">
        <img src="assets/network-asset-asynccode-20250502115449-lndh6c9.png" alt="Runtime State" style="max-width: 100%;">
    </div>
</div>

　　游戏运行时，相关初始化设置会被自动更新至运行时的实体状态，如右图所示。

### 通过 Flow 更新实体状态

　　当前 SyPlugins 提供了通过 Flow 在运行时设置实体状态的方案

　　我们可以通过多种方式执行 Flow，如在 GameInstance、World 中运行，这里提供一种快捷解决方案，即在 BPActor 的 BeginPlay 快速注册

![](assets/network-asset-asynccode-20250502115449-4ag170a.png)

　　当前提供了两组进行快速 State 操作的节点

![](assets/network-asset-asynccode-20250502115450-e7nxtsb.png)

　　如图所示，节点支持为符合条件的实体添加指定状态修改

　　其中 SetupInteractionState 节点封装了对于可交互状态和交互内容的修改，配置更加便捷，而 ApplyStateOp 支持快速修改单个状态

![](assets/network-asset-asynccode-20250502115450-3u8ak2j.png)

　　如图所示，Flow 执行 SetupInteractionState 节点后，指定状态被附加到了角色实体的 Global 状态上，并和 Local 数据覆盖合并，生成最终状态

　　（注：UE 又出了个逆天 Bug，第二次调用状态合并时，Local 状态在代码中会丢失，集合变成 Empty，但面板显示正常。该 Bug 导致默认状态配置不稳定，待抢救）

## 三、结合 FlowExtension 测试任务运行

　　通过进入 内容浏览器 -\> Plugins -\> SyPluginsImpl Content -\> Maps -\> Level\_SyTest 可查看示例场景，以及场景中任务的基本运作

　　基础任务和显示参考 [ModularityPractice](https://github.com/Variann/ModularityPractice) 中的示例工程

　　通过定义 /SyPluginsImpl/Quests/QuestAssets/Q\_SyTest 文件，我们定义了几个简单任务，并在 Flow 中串联管理状态和更新任务条件

　　Flow 文件见 /SyPluginsImpl/Quests/FlowAssets/FA\_SyPluginTest

![](assets/network-asset-asynccode-20250502115450-ixh2ucf.png)

　　基础逻辑为：

* 关注任务的多个状态：接取、完成
* 基于流定义任务状态下的逻辑，包含监听触发、设置状态等
* 基于状态管理位于实体身上的触发入口，推进任务流程

　　在示例蓝图中我们简易处理了两组不同的监听逻辑

* 触发器监听
* 交互监听

　　他们均为通过 Actor 身上运行的 蓝图 / 脚本 / Flow 等通过 SyEntity 暴露的接口，使用 SyMessage 组件，基于 SyIdentity 定义的身份发送。

![](assets/network-asset-asynccode-20250502115450-3uyd0k5.png)

　　（下图为实体通过 SyInteractComponent 触发 Flow，并通过 SendInteractEnd 节点发送交互完成消息，类似交互开始消息则在 C++ 中处理）

　　（上图为相对草率的 TriggerActor 实现，理想情况下发消息前还应该检查自身状态是否被激活 hhh）

　　基于这一消息总线，简单实现了 实体 / 状态 / 交互 之间的串联，初步定义了一个易于拓展的插件框架，有朋友能一起交流使用

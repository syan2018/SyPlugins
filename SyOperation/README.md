# SyOperation Module

## 概述

`SyOperation` 模块是 `SyPlugins` 系统中的一个核心模块，旨在提供一个通用的、数据驱动的框架来**定义状态变更操作的意图**。它不负责执行操作，而是提供标准化的数据结构 `FSyOperation` 来描述一个操作的来源、目标和具体的修改内容，以便其他系统（如流程控制、游戏逻辑模块）可以统一处理。

## 核心概念

1.  **操作意图 (`FSyOperation`)**:
    *   核心结构体 `FSyOperation` 代表一个完整的状态变更意图。
    *   它由三部分组成：
        *   `FSyOperationSource`: 定义操作的发起者或来源（例如：技能、交互、系统事件）。使用 `GameplayTag` (`SourceTypeTag`) 对来源类型进行分类，并可通过 `FSyInstancedStruct` (`Parameters`) 携带来源相关的参数。
        *   `FSyOperationModifier`: 定义操作希望产生的具体状态修改效果。它内部包含一个 `FSyStateParameterSet` (来自 `SyStateCore` 模块)，用于存储一个或多个目标状态标签 (`StateTag`) 及其对应的参数 (`FSyInstancedStruct`)。
        *   `FSyOperationTarget`: 定义操作的目标（例如：特定实体、区域、全局状态）。使用 `GameplayTag` (`TargetTypeTag`) 对目标类型进行分类，并可通过 `FSyInstancedStruct` (`Parameters`) 携带目标相关的参数。

2.  **数据驱动配置**:
    *   **`GameplayTag`**: 广泛用于对 `Source`, `Target` 类型以及 `Modifier` 中的状态进行分类和识别。
    *   **`FSyInstancedStruct` (来自 `SyCore`)**: 用于携带与特定 `GameplayTag` 关联的参数数据。这允许操作的细节（如伤害值、状态效果持续时间等）通过数据进行配置，而不是硬编码。

3.  **状态修改集 (`FSyStateParameterSet` - 来自 `SyStateCore`)**:
    *   `FSyOperationModifier` 使用 `FSyStateParameterSet` 来聚合本次操作想要应用的所有状态修改。每个修改由一个目标状态的 `GameplayTag` 和一个包含具体参数的 `FSyInstancedStruct` 组成。

## 主要特性

1.  **标准化的操作意图定义**: 提供统一的数据结构来描述各种状态变更操作。
2.  **数据驱动**: 操作的类型和参数可以通过 `GameplayTag` 和 `FSyInstancedStruct` 进行灵活配置。
3.  **编辑器增强**:
    *   提供 `FSyOperationDetailsCustomization`, 在虚幻编辑器中自定义 `FSyOperation` 的属性面板。
    *   当修改 `SourceTypeTag` 或 `TargetTypeTag` 时, 可以自动更新对应的 `Parameters` 结构体类型（需要配合 `TagMetadata` 配置）。
4.  **可扩展性**: 可以通过定义新的 `GameplayTag` 和对应的参数结构体 (继承自特定基类或直接使用) 来轻松扩展支持新的操作来源、目标和状态修改类型。

## 与其他模块的协作

*   `SyOperation` **定义意图**: 本模块只负责定义 `FSyOperation` 数据结构。
*   **其他模块处理操作**: 实际的操作执行逻辑通常位于其他模块，例如：
    *   `SyFlowImpl` 可能包含处理特定 `FSyOperation` 的流程节点。
    *   游戏逻辑模块（如技能系统、交互系统）可能会创建 `FSyOperation` 实例，并通过消息系统或其他机制将其分发出去。
    *   处理模块接收到 `FSyOperation` 后，会解析其内容，并调用 `SyStateCore` 或 `SyEntity` 提供的接口来实际应用状态变更。

## 使用方法

### 1. (可选) 配置Tag与参数类型映射 (用于编辑器增强)

如果希望在编辑器中根据选择的 `SourceTypeTag` 或 `TargetTypeTag` 自动更新参数类型，需要配置 `TagMetadataCollection`：

1.  在编辑器中创建或打开 `TagMetadataCollection` 资产。
2.  为操作来源/目标相关的 `GameplayTag` 添加 `UTagMetadata` (或其子类，具体取决于你的实现，例如旧版本示例中的 `USyOperationParamSchemaMetadata`)。
3.  在元数据中指定与该 Tag 关联的参数结构体类型 (必须是 `USTRUCT`)。

```cpp
// 示例：为交互来源配置Tag元数据
// Tag: Source.Interaction
// Metadata: (某个 UTagMetadata 子类)
//   ParameterStructType: FInteractionSourceParams
```

### 2. 创建操作意图

```cpp
#include \"OperationTypes.h\"
#include \"OperationParams.h\" // 包含参数结构体定义
#include \"SyCore/Public/Foundation/Utilities/SyInstancedStruct.h\"
#include \"GameplayTagContainer.h\"
#include \"StateParameterTypes.h\" // 包含 FSyStateParameterSet

// 创建一个操作意图
FSyOperation MyOperation;
MyOperation.OperationId = FGuid::NewGuid(); // 分配唯一ID

// 1. 设置来源 (例如：来自一个名为 'Ability.Fireball' 的技能)
FGameplayTag SourceTag = FGameplayTag::RequestGameplayTag(\"Source.Ability.Fireball\");
// (假设火球技能没有特定来源参数，或使用默认构造)
MyOperation.Source = FSyOperationSource(SourceTag);

// 2. 设置修饰器 (例如：施加燃烧状态 'State.Debuff.Burning')
FGameplayTag BurningStateTag = FGameplayTag::RequestGameplayTag(\"State.Debuff.Burning\");
// (假设燃烧状态需要参数 FBurningParams)
// FBurningParams BurningParams(10.0f, 5.0f); // 假设定义了燃烧参数结构体
// FSyInstancedStruct BurningParamsStruct;
// BurningParamsStruct.InitializeAs<FBurningParams>(BurningParams); // 用具体参数初始化
FSyStateParameterSet ModifierParams;
// ModifierParams.AddStateParam(BurningStateTag, BurningParamsStruct);
// 如果没有参数，可以这样添加:
ModifierParams.AddStateParam(BurningStateTag, FSyInstancedStruct());


MyOperation.Modifier = FSyOperationModifier(ModifierParams);

// 3. 设置目标 (例如：一个类型为 'EntityType.Character' 的敌人)
FGameplayTag TargetTag = FGameplayTag::RequestGameplayTag(\"Target.EntityType.Character\");
// (假设目标需要一个实体ID参数 FTargetEntityParams)
// FTargetEntityParams TargetParams(EnemyActor->GetUniqueID()); // 假设定义了目标参数
// FSyInstancedStruct TargetParamsStruct(TargetParams);
// MyOperation.Target = FSyOperationTarget(TargetTag, TargetParamsStruct);
// 如果目标类型本身足够，或参数在其他地方处理:
MyOperation.Target = FSyOperationTarget(TargetTag);


// 4. 将 MyOperation 分发给处理系统 (例如通过消息总线或直接调用)
// MessageSubsystem->BroadcastMessage(OperationChannel, MyOperation);
```

### 3. 自定义参数结构体

```cpp
#include \"CoreMinimal.h\"
#include \"OperationParams.generated.h\" // 或其他合适的文件

// 创建自定义参数结构体
USTRUCT(BlueprintType)
struct FMyCustomSourceParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PowerLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SourceName;
};

// 然后可以在创建 FSyOperationSource 时使用:
// FMyCustomSourceParams MyParams;
// MyParams.PowerLevel = 10.f;
// MyParams.SourceName = TEXT(\"MegaBlast\");
// FSyInstancedStruct ParamsStruct;
// ParamsStruct.InitializeAs<FMyCustomSourceParams>(MyParams);
// MyOperation.Source = FSyOperationSource(SourceTag, ParamsStruct);

// 如果希望编辑器支持自动类型切换，需要配置 TagMetadataCollection
```

## 最佳实践

1.  **参数结构体设计**:
    *   保持结构体功能单一，避免过于庞大。
    *   为编辑器暴露的属性添加清晰的 `Tooltip`。
    *   考虑结构体的默认值。
2.  **Tag 组织**:
    *   使用一致且有意义的 `GameplayTag` 层级结构（例如 `Source.Skill.Melee`, `Target.EntityType.Character`, `State.Buff.Haste`）。
    *   在项目的 `GameplayTags` 管理器中集中定义和管理 Tags。
3.  **关注点分离**:
    *   `SyOperation` 只定义"什么"操作，具体的"如何"执行由其他系统负责。
    *   参数结构体应只包含数据，不包含逻辑。
4.  **编辑器配置 (如果使用)**:
    *   确保 `TagMetadataCollection` 配置正确且最新。
    *   利用自定义详情面板简化设计师的配置工作。

## 注意事项

1.  参数结构体的修改（添加/删除/重命名属性）可能会影响已保存的 `FSyOperation` 数据。
2.  编辑器相关的代码（如 `FSyOperationDetailsCustomization`）仅在编辑器构建中有效。
3.  如果参数需要在网络间同步或保存到磁盘，请确保结构体及其包含的类型支持序列化。
4.  `FSyInstancedStruct` 的使用需要确保其内部结构体指针在需要时是有效的。

## 依赖关系

*   **`SyCore`**: 提供 `FSyInstancedStruct` 等基础工具和概念。
*   **`SyStateCore`**: 提供 `FSyStateParameterSet` 用于定义状态修改，以及可能的状态标签 (`StateTag`) 概念。
*   **`GameplayTags`**: Unreal Engine 模块，提供核心的标签系统。
*   **`StructUtils`**: Unreal Engine 模块，`FInstancedStruct` 的基础。
*   **`TagMetadata` (可选)**: Unreal Engine 插件，用于编辑器中 Tag 与数据的关联（如果使用该机制进行参数类型推断）。

## 未来计划

(根据实际情况填写)
*   ...
*   ...
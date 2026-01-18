
**SyGameplay 交互模块设计文档 V1.0**

暂时未预期实现，预期工期略长，仅作讨论存档

初期设计计划为新增可用交互管理组件，但可用交互由InteractionComp事件注册，而非主动收集交叉验证

但需要一定程度上拓展可用交互管理组件，并让UI管理器监听处理显示

**1. 简介与目标**

**1.1. 目标**
构建一个灵活、可扩展、高内聚、低耦合的交互系统，用于处理游戏中玩家或其他实体与可交互对象之间的互动。该系统旨在：

*   支持多种交互类型（瞬时、长按、菜单选择等）。
*   支持不同的交互范围和形状。
*   解耦交互检测、输入处理、交互逻辑执行和 UI 反馈。
*   易于添加新的可交互对象和交互行为。
*   与现有的 `SyStateComponent` 和 `SyEntityComponent` 系统良好集成。

**1.2. 核心概念**

*   **交互物 (Interactable):** 游戏世界中可以被交互的 Actor 或其上的组件 (e.g., 门、NPC、开关)。由 `USyInteractionComponent` 表示。
*   **交互者 (Interactor):** 能够发起交互的 Actor (e.g., 玩家角色、AI)。通常会携带 `USyInteractionDetectorComponent` 和 `USyInteractionInputComponent`。
*   **检测 (Detection):** 交互者发现其范围内可交互物的过程。
*   **焦点 (Focus):** 在多个可交互物中，交互者当前主要关注或准备交互的目标。
*   **执行 (Execution):** 触发交互物定义的具体逻辑的过程。

**2. 系统架构**

交互系统主要由以下几个核心组件构成，它们通过事件和接口进行通信：

```mermaid
graph TD
    subgraph Interactor (e.g., Player Pawn)
        Input[UEnhancedInputComponent] --> InputHandler[USyInteractionInputComponent]
        InputHandler -- Gets Focus --> Detector[USyInteractionDetectorComponent]
        InputHandler -- Calls ExecuteInteraction --> FocusedInteractable(Focused USyInteractionComponent)
        Detector -- Detects --> AvailableInteractables[Available USyInteractionComponents]
        Detector -- Selects Focus --> FocusedInteractable
        Detector -- Notifies Changes --> UIManager[USyInteractionUIManager]
    end

    subgraph Interactable (e.g., Door Actor)
        InteractableComp[USyInteractionComponent]
        InteractableComp -- Uses --> StateComp[USyStateComponent]
        InteractableComp -- Sends Message --> EntityComp[USyEntityComponent]
    end

    subgraph UI System
        UIManager -- Updates --> InteractionWidget[UMG Widget]
    end

    InputHandler -. Listens for Input Actions .-> Input
    Detector -. Overlaps/Traces .-> InteractableComp
    UIManager -. Listens for Focus Changes .-> Detector
    InteractableComp -. Checks State .-> StateComp
    InteractableComp -. Sends Event .-> EntityComp
```

**3. 组件详解**

**3.1. `USyInteractionComponent` (交互物组件)**

*   **父类:** `UActorComponent`
*   **职责:**
    *   标识其所属 Actor 是可交互的。
    *   定义交互的具体属性和行为。
    *   管理自身的交互状态（通过 `USyStateComponent` 或内部逻辑）。
    *   响应交互者的交互请求。
*   **关键属性 (`UPROPERTY`)**:
    *   `TObjectPtr<UShapeComponent> InteractionShape`: (可选) 定义精确的交互范围形状 (替代简单的半径检查)。如果为空，则可能依赖检测器的粗略范围或 `InteractionRadius`。
    *   `float InteractionRadius`: (如果 `InteractionShape` 为空) 定义简单的球形交互半径。
    *   `FGameplayTag InteractionActionTag`: 触发此交互所需的输入动作标签 (e.g., `Input.Interact.Use`, `Input.Interact.Examine`)。
    *   `ESyInteractionType InteractionType`: 交互类型 (e.g., `Instant`, `Hold`, `Menu`)。
    *   `float InteractionDuration`: (仅用于 `Hold` 类型) 需要按住的时长。
    *   `FText InteractionPromptText`: 显示给玩家的交互提示文本 (e.g., "按 E 打开")。
    *   `FGameplayTag RequiredStateTag`: (可选) 允许交互所需的状态标签 (e.g., `State.Interactable`)。组件会监听 `SyStateComponent` 的变化。
    *   `bool bEnabled`: 组件内部的启用状态，可由 `RequiredStateTag` 或外部逻辑控制。
*   **关键方法 (`UFUNCTION`)**:
    *   `bool CanInteract(AActor* Interactor) const`: 检查指定的交互者是否满足所有交互条件（距离/范围、状态标签、组件是否启用等）。
    *   `void ExecuteInteraction(AActor* Interactor)`: 执行瞬时交互的主要逻辑（通常是向 `SyEntityComponent` 发送消息）。
    *   `void StartInteraction(AActor* Interactor)`: (用于 `Hold` 或复杂交互) 开始交互过程。
    *   `void UpdateInteraction(AActor* Interactor, float DeltaTime)`: (用于 `Hold` 或复杂交互) 更新交互进度。
    *   `void CompleteInteraction(AActor* Interactor)`: (用于 `Hold` 或复杂交互) 成功完成交互。
    *   `void CancelInteraction(AActor* Interactor)`: (用于 `Hold` 或复杂交互) 中断交互。
    *   `FText GetInteractionPrompt() const`: 获取交互提示文本。
    *   `FGameplayTag GetInteractionActionTag() const`: 获取触发此交互所需的输入标签。
    *   `ESyInteractionType GetInteractionType() const`: 获取交互类型。
*   **关键事件 (Delegates)**:
    *   `OnInteractionExecuted`: (可选) 交互成功执行时广播。
    *   `OnInteractionCompleted`: (可选, 用于 Hold/Complex) 交互成功完成时广播。
    *   `OnInteractionStateChanged`: (可选) 当 `bEnabled` 或由 `RequiredStateTag` 控制的可交互状态改变时广播。
*   **依赖:** `UShapeComponent` (可选), `USyStateComponent` (可选), `USyEntityComponent` (推荐)。

**3.2. `USyInteractionDetectorComponent` (交互检测器组件)**

*   **父类:** `UActorComponent`
*   **职责:**
    *   检测其 Owner 周围的可交互对象 (`USyInteractionComponent`)。
    *   维护一个当前可用且满足条件的交互物列表。
    *   根据策略选择一个焦点交互物。
    *   通知其他系统（主要是 UI 和输入）有关可用交互物和焦点变化的信息。
*   **关键属性 (`UPROPERTY`)**:
    *   `TObjectPtr<UShapeComponent> DetectionShape`: 用于粗略检测的主要范围组件 (e.g., `USphereComponent`)。
    *   `ESyDetectionMethod DetectionMethod`: 检测方式 (e.g., `Overlap`, `Trace`)。
    *   `ESyFocusStrategy FocusStrategy`: 选择焦点的策略 (e.g., `ClosestInDetectionShape`, `ClosestToCenterScreen`, `ManualCycle`)。
    *   `TArray<TWeakObjectPtr<USyInteractionComponent>> AvailableInteractions`: 当前范围内且 `CanInteract()` 返回 true 的交互物列表。
    *   `TWeakObjectPtr<USyInteractionComponent> FocusedInteraction`: 当前选定的焦点交互物。
    *   `float TraceDistance`: (用于 `Trace` 检测方式) 射线检测距离。
    *   `float CenterScreenFocusRadius`: (用于 `ClosestToCenterScreen`) 屏幕中心检测区域半径。
*   **关键方法 (`UFUNCTION`)**:
    *   `void TickComponent(...)`: 主要更新逻辑发生的地方：执行检测、更新 `AvailableInteractions`、根据策略更新 `FocusedInteraction`、触发事件。
    *   `USyInteractionComponent* GetFocusedInteraction() const`: 获取当前焦点交互物。
    *   `const TArray<TWeakObjectPtr<USyInteractionComponent>>& GetAvailableInteractions() const`: 获取可用交互物列表。
    *   `void CycleFocus()`: (用于 `ManualCycle` 策略) 手动切换焦点。
*   **关键事件 (Delegates)**:
    *   `OnFocusedInteractionChanged(USyInteractionComponent* NewFocus)`: 当焦点交互物改变时广播。
    *   `OnAvailableInteractionsChanged(const TArray<TWeakObjectPtr<USyInteractionComponent>>& NewAvailableList)`: 当可用交互物列表发生变化时广播。
*   **依赖:** `UShapeComponent`。

**3.3. `USyInteractionInputComponent` (交互输入处理器)**

*   **父类:** `UActorComponent` (或逻辑可以集成到 PlayerController / PlayerState)
*   **职责:**
    *   监听与交互相关的 `UEnhancedInputComponent` 的输入事件。
    *   根据输入的 `Action` 和当前 `FocusedInteraction` 的 `InteractionActionTag` 进行匹配。
    *   调用 `FocusedInteraction` 上的相应方法 (`Execute`, `Start`, `Complete`, `Cancel`)。
*   **关键属性 (`UPROPERTY`)**:
    *   `TMap<TObjectPtr<UInputAction>, FGameplayTag> InteractionActionMappings`: 将具体的 Input Action 映射到交互动作标签。
    *   `TWeakObjectPtr<USyInteractionDetectorComponent> DetectorComponent`: 缓存对 Owner 身上的检测器组件的引用。
*   **关键方法 (`UFUNCTION`)**:
    *   `void InitializeInputComponent(UEnhancedInputComponent* PlayerInputComponent)`: 绑定输入动作到处理函数。
    *   `void HandleInputAction_Triggered(const FInputActionValue& Value, FGameplayTag ActionTag)`: 处理瞬时触发的输入。
    *   `void HandleInputAction_Started(const FInputActionValue& Value, FGameplayTag ActionTag)`: 处理开始按下的输入 (用于 Hold)。
    *   `void HandleInputAction_Completed(const FInputActionValue& Value, FGameplayTag ActionTag)`: 处理释放输入的事件 (用于 Hold 完成或取消)。
    *   `void HandleInputAction_Canceled(const FInputActionValue& Value, FGameplayTag ActionTag)`: 处理输入取消事件 (用于 Hold 取消)。
*   **依赖:** `UEnhancedInputComponent`, `USyInteractionDetectorComponent`。

**3.4. `USyInteractionUIManager` (交互 UI 管理器)**

*   **父类:** `UActorComponent` 或 `UGameInstanceSubsystem`
*   **职责:**
    *   监听 `USyInteractionDetectorComponent` 的事件。
    *   根据焦点交互物的信息，创建、显示、更新或隐藏交互提示 UI (UMG Widget)。
    *   (可选) 显示多个可用交互物的指示器。
*   **关键属性 (`UPROPERTY`)**:
    *   `TSubclassOf<UUserWidget> InteractionWidgetClass`: 用于显示交互提示的 Widget 类。
    *   `TWeakObjectPtr<USyInteractionDetectorComponent> DetectorComponent`: 缓存对检测器的引用。
    *   `TObjectPtr<UUserWidget> CurrentInteractionWidget`: 当前显示的 Widget 实例。
*   **关键方法 (`UFUNCTION`)**:
    *   `void HandleFocusedInteractionChanged(USyInteractionComponent* NewFocus)`: 响应焦点变化，更新 UI。
    *   `void HandleAvailableInteractionsChanged(...)`: (可选) 响应可用列表变化，可能用于显示多个提示。
*   **依赖:** `USyInteractionDetectorComponent`, `UMG`。

**4. 数据结构**

**4.1. `ESyInteractionType` (枚举)**

```cpp
UENUM(BlueprintType)
enum class ESyInteractionType : uint8
{
    Instant UMETA(DisplayName="Instant Use"),     // 按下立即触发
    Hold    UMETA(DisplayName="Hold Use"),        // 按住持续触发，直到完成或取消
    Menu    UMETA(DisplayName="Context Menu"),    // 打开一个上下文菜单 (未来扩展)
    Toggle  UMETA(DisplayName="Toggle State")     // 切换状态 (未来扩展)
};
```

**4.2. `ESyDetectionMethod` (枚举)**

```cpp
UENUM(BlueprintType)
enum class ESyDetectionMethod : uint8
{
    Overlap UMETA(DisplayName="Overlap Detection"), // 基于 Shape Overlap
    Trace   UMETA(DisplayName="Trace Detection")    // 基于射线或形状扫描
};
```

**4.3. `ESyFocusStrategy` (枚举)**

```cpp
UENUM(BlueprintType)
enum class ESyFocusStrategy : uint8
{
    ClosestInDetectionShape UMETA(DisplayName="Closest In Shape"), // 检测范围内最近的
    ClosestToCenterScreen UMETA(DisplayName="Closest To Center Screen"), // 离屏幕中心最近的
    ManualCycle           UMETA(DisplayName="Manual Cycle")           // 手动切换
};
```

**5. 工作流程示例**

**5.1. 检测与焦点更新**
1.  `DetectorComponent::TickComponent`:
    *   使用 `DetectionShape` 或 Trace 执行检测。
    *   对每个检测到的 Actor，获取 `USyInteractionComponent`。
    *   调用 `Interactable->CanInteract(OwnerActor)` 过滤。
    *   更新 `AvailableInteractions` 列表。
    *   如果列表变化，广播 `OnAvailableInteractionsChanged`。
    *   根据 `FocusStrategy` 从 `AvailableInteractions` 中选择新的 `FocusedInteraction`。
    *   如果 `FocusedInteraction` 改变，广播 `OnFocusedInteractionChanged`。

**5.2. UI 更新**
1.  `UIManager::HandleFocusedInteractionChanged`:
    *   如果 `NewFocus` 有效：
        *   获取 `NewFocus->GetInteractionPrompt()`。
        *   如果 `CurrentInteractionWidget` 不存在或类不对，创建/替换 Widget。
        *   更新 Widget 的文本和可见性。
    *   如果 `NewFocus` 为空：
        *   隐藏或销毁 `CurrentInteractionWidget`。

**5.3. 瞬时交互执行**
1.  玩家按下绑定到 `Input.Interact.Use` (Tag) 的键。
2.  `InputHandler::HandleInputAction_Triggered` 被调用，传入 `ActionTag`。
3.  `InputHandler` 获取 `DetectorComponent->GetFocusedInteraction()`。
4.  如果 `FocusedInteraction` 有效，并且 `FocusedInteraction->GetInteractionActionTag() == ActionTag` 并且 `FocusedInteraction->GetInteractionType() == Instant`：
5.  调用 `FocusedInteraction->ExecuteInteraction(OwnerActor)`。
6.  `InteractionComponent::ExecuteInteraction` 内部逻辑执行（例如，向 `SyEntityComponent` 发送 `Event.Interaction` 消息）。

**5.4. 长按交互执行**
1.  玩家按下绑定到 `Input.Interact.HoldUse` 的键。
2.  `InputHandler::HandleInputAction_Started` 被调用。
3.  `InputHandler` 获取焦点，检查 Tag 和 Type (`Hold`)，调用 `FocusedInteraction->StartInteraction()`。
4.  `InteractionComponent::StartInteraction` 初始化计时器或状态。
5.  (每 Tick) 可能需要 `InteractionComponent::UpdateInteraction()`。
6.  玩家持续按住达到 `InteractionDuration` 后，释放按键。
7.  `InputHandler::HandleInputAction_Completed` 被调用。
8.  `InputHandler` 获取焦点，检查状态，调用 `FocusedInteraction->CompleteInteraction()`。
9.  `InteractionComponent::CompleteInteraction` 执行完成逻辑，发送消息。
10. (如果玩家中途释放或取消) `HandleInputAction_Canceled` 或 `Completed` (根据 Enhanced Input 配置) 被调用，触发 `FocusedInteraction->CancelInteraction()`。

**6. 集成点**

*   **输入系统:** 通过 `UEnhancedInputComponent` 和 `UInputAction` 进行绑定。
*   **UI 系统:** 通过 `UMG` 和 `UUserWidget` 显示反馈。
*   **状态系统:** `USyInteractionComponent` 可选地依赖 `USyStateComponent` 来确定可交互状态。
*   **消息系统:** `USyInteractionComponent` 在成功交互后，通过 `USyEntityComponent` 发送 Gameplay 消息。

**7. 扩展性**

*   **新交互类型:** 扩展 `ESyInteractionType`，并在 `USyInteractionComponent` 和 `USyInteractionInputComponent` 中添加处理逻辑。
*   **新检测/焦点策略:** 扩展 `ESyDetectionMethod` / `ESyFocusStrategy`，并在 `USyInteractionDetectorComponent` 中实现相应逻辑。
*   **自定义交互行为:** 创建 `USyInteractionComponent` 的子类，覆盖其 `Execute/Start/Complete/Cancel` 等方法。
*   **复杂 UI:** `USyInteractionUIManager` 可以扩展以支持显示多个可用交互物的列表 UI。

**8. 初步实现计划**

1.  定义核心数据结构 (`ESyInteractionType`, `ESyFocusStrategy` 等)。
2.  实现 `USyInteractionComponent` 基础功能 (属性、`CanInteract`, `ExecuteInteraction`, 简单的距离检查)。
3.  实现 `USyInteractionDetectorComponent` 基础功能 (Overlap 检测、`ClosestInDetectionShape` 策略、`OnFocusedInteractionChanged` 事件)。
4.  实现 `USyInteractionInputComponent` 基础功能 (绑定一个瞬时 Action、调用 `ExecuteInteraction`)。
5.  实现 `USyInteractionUIManager` 基础功能 (监听焦点变化、显示/隐藏简单文本 Widget)。
6.  进行集成测试，确保基础流程跑通。
7.  逐步添加 Hold 类型、其他策略、状态依赖等高级功能。

**9. 注意事项**

*   性能：高频的 Trace 或复杂的 Overlap 查询需要注意性能优化。`DetectorComponent` 的 Tick 频率可以适当调整。
*   网络同步：如果游戏需要多人联机，所有交互状态和事件都需要考虑网络复制。`ExecuteInteraction` 等关键操作应在服务器上执行或验证。
*   资源引用：注意使用 `TWeakObjectPtr` 来避免循环引用，特别是在 `DetectorComponent` 缓存交互物时。

这份文档提供了一个相对完整的蓝图。我们可以根据实际开发中的需要进行调整和细化。

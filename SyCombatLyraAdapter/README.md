# SyCombatLyraAdapter - SyCombat 的 Lyra/GAS 适配层

## 定位
该插件把 `SyCombat` 的 **ActionRequest** 转成 GAS 可消费的 **GameplayEvent**，从而复用 Lyra/GAS 的预测、复制、回滚与 AbilitySet 生态。

## 当前内容（原型骨架）
- `USyCombatGASAbilityDriverComponent`
  - 监听 `USyCombatComponent::OnActionRequested`
  - 按 `ActionToGameplayEventTag` 映射向 Actor 发送 GameplayEvent（建议由 AbilitySet/Ability 监听事件触发）

## 典型用法
1. 角色挂载：
   - `USyEntityComponent`
   - `USyCombatComponent`（SyCombat）
   - `USyCombatGASAbilityDriverComponent`（本插件）
2. 配置 `ActionTag -> GameplayEventTag`
3. 让对应 Ability 在 GAS 侧监听该 GameplayEventTag，并配置 Tag Gate（推荐）


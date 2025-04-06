# SyOperation Module

## 概述

SyOperation模块是SyPlugins系统中的一个核心模块，用于定义状态变更操作的数据结构和参数定义机制。它提供了一套完整的工具，用于创建、配置和管理游戏中的状态变更操作。

## 核心功能

1. **操作数据结构**
   - `FSyOperation`: 完整的状态变更操作定义
   - `FSyOperationSource`: 操作来源定义
   - `FSyOperationModifier`: 操作修饰器定义
   - `FSyOperationTarget`: 操作目标定义

2. **参数Schema系统**
   - `USyOperationParamSchemaMetadata`: 通过TagMetadata插件将GameplayTag与参数结构体类型关联
   - 支持在TagMetadataCollection中配置Tag与参数类型的映射

3. **常用参数结构体**
   - `FDamageParams`: 基础伤害参数
   - `FBurningDamageParams`: 燃烧伤害参数
   - `FApplyStatusParams`: 状态效果参数
   - `FInteractionSourceParams`: 交互来源参数

4. **编辑器支持**
   - 自定义详情面板，提供更好的编辑体验
   - Tag变更时自动更新参数结构体类型
   - 参数验证和错误提示

## 使用方法

### 1. 配置Tag与参数类型映射

1. 在编辑器中创建或打开TagMetadataCollection资产
2. 为操作相关的Tag添加USyOperationParamSchemaMetadata
3. 在Schema中设置对应的参数结构体类型

```cpp
// 示例：配置伤害操作的Tag
// Tag: Modifier.Damage.Physical
// SchemaMetadata类型: USyOperationParamSchemaMetadata
// ParameterStructType: FDamageParams
```

### 2. 创建操作

```cpp
// 创建物理伤害操作
FSyOperation DamageOperation;

// 设置来源（比如技能）
DamageOperation.Source = FSyOperationSource(
    FGameplayTag::RequestGameplayTag("Source.Skill.Attack"),
    FSyInstancedStruct(FInteractionSourceParams())
);

// 设置修饰器（伤害类型和参数）
FDamageParams DamageParams(50.0f, FGameplayTag::RequestGameplayTag("DamageType.Physical"));
DamageOperation.Modifier = FSyOperationModifier(
    FGameplayTag::RequestGameplayTag("Modifier.Damage.Physical"),
    FSyInstancedStruct(DamageParams)
);

// 设置目标
DamageOperation.Target = FSyOperationTarget(
    FGameplayTag::RequestGameplayTag("EntityType.Character"),
    TargetEntityId
);
```

### 3. 自定义参数结构体

```cpp
// 创建自定义参数结构体
USTRUCT(BlueprintType)
struct FCustomOperationParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag Type;
};

// 在TagMetadataCollection中配置
// Tag: Modifier.Custom
// SchemaMetadata: USyOperationParamSchemaMetadata
// ParameterStructType: FCustomOperationParams
```

## 最佳实践

1. **参数结构体设计**
   - 保持结构体简单明确
   - 使用合适的默认值
   - 添加必要的编辑器元数据

2. **Tag组织**
   - 使用清晰的层级结构
   - 为不同类型的操作使用不同的前缀
   - 保持Tag命名的一致性

3. **Schema配置**
   - 在项目初期完成基础Tag和Schema的配置
   - 为每个操作类型创建专门的参数结构体
   - 定期检查和更新Schema配置

4. **编辑器使用**
   - 利用自定义详情面板简化配置
   - 注意参数类型的自动更新
   - 关注验证错误提示

## 注意事项

1. 确保所有使用的Tag都在TagMetadataCollection中有对应的Schema配置
2. 参数结构体的修改可能影响已有的操作配置
3. 编辑器模块仅在编辑器中可用
4. 注意参数结构体的序列化和网络复制支持

## 依赖关系

- SyCore
- GameplayTags
- TagMetadata
- StructUtils

## 未来计划

1. 添加更多常用参数结构体
2. 增强编辑器支持
3. 添加参数验证系统
4. 提供更多辅助工具
# SyUtils Plugin

个人收藏的零碎功能蓝图库，主要用于提供通用反射、数据访问等功能。

## 模块说明

- **SyUtils**: 核心运行时模块，包含蓝图函数库。

## 功能特性

### 1. 通用结构体属性访问 (Reflection)
允许在蓝图中通过变量名（字符串）直接获取任意结构体（Struct）的成员变量值。

**蓝图节点:**
- `Get Struct Property (String)`: 获取任意类型属性并转换为字符串。
- `Get Struct Property (Int)`: 获取整型属性。
- `Get Struct Property (Float)`: 获取浮点型属性。
- `Get Struct Property (Struct)`: **(新增)** 获取嵌套的结构体属性。输出引脚支持“自主选择”类型（连接任意 Struct 变量即可决定输出类型）。

**使用方法:**
将任意 Struct 连入 `Target Struct` 引脚，输入变量名 `Property Name` 即可。对于 `(Struct)` 版本的节点，你需要将目标类型的 Struct 变量（或 Make Struct 节点）连接到输出引脚，以便告诉节点你期望的类型。

### 2. DataTable 动态访问
允许通过 Row Name (行名) 和 Column Name (列名/变量名) 动态获取数据表中的值，无需预先 Cast 为特定行结构体。

**蓝图节点:**
- `Get Data Table Value (String)`
- `Get Data Table Value (Int)`
- `Get Data Table Value (Float)`
- `Get Data Table Value (Struct)`: **(新增)** 直接获取 DataTable 中某一列的 Struct 数据。

## 依赖
- Core
- CoreUObject
- Engine

## 更新日志
- **v1.1**: 新增对 Struct 类型属性的动态访问支持（GetStructPropertyAsStruct, GetDataTableValueAsStruct）。
- **v1.0**: 初始版本，支持 Struct 和 DataTable 的动态属性访问（基础类型）。

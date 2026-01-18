#pragma once

#include "CoreMinimal.h"
#include "Foundation/SyInstancedStruct.h" // 包含基类定义
#include "StructUtils/InstancedStruct.h" // 包含 FInstancedStruct
#include "ListMetadataValueTypes.generated.h"

/**
 * @brief 通用列表参数的基类。
 * 子类需要实现具体的列表存储和聚合逻辑。
 * 提供虚函数接口供内部系统（如状态聚合）调用。
 */
USTRUCT() // 仍然是 USTRUCT
struct SYCORE_API FSyListParameterBase : public FSyBaseInstancedStruct
{
    GENERATED_BODY()

    /** 用于定义子类的对象类型 */
    TObjectPtr<const UScriptStruct> ScriptStruct = nullptr;

    /**
     * @brief [Internal C++] 获取此列表结构实际存储项的数组引用（子类实现）。
     * 非 UFUNCTION，仅供 C++ 内部聚合逻辑使用。
     * @return 对内部列表项数组的可变引用。
     */
    virtual TArray<FInstancedStruct>& GetListItemsInternal()
    {
        // 基类返回静态空数组引用，防止直接使用基类时出错
        static TArray<FInstancedStruct> EmptyArray;
        UE_LOG(LogTemp, Warning, TEXT("GetListItemsInternal called on FSyListParameterBase itself. Returning empty array. Ensure you are using a derived class instance."));
        return EmptyArray;
    }

    /**
     * @brief [Internal C++] 获取此列表结构实际存储项的数组常量引用（子类实现）。
     * 非 UFUNCTION，仅供 C++ 内部聚合逻辑使用。
     * @return 对内部列表项数组的常量引用。
     */
    virtual const TArray<FInstancedStruct>& GetListItemsInternal() const
    {
        static const TArray<FInstancedStruct> EmptyArray;
        UE_LOG(LogTemp, Warning, TEXT("GetListItemsInternal (const) called on FSyListParameterBase itself. Returning empty array. Ensure you are using a derived class instance."));
        return EmptyArray;
    }

    /**
     * @brief [Internal C++] 将源列表项聚合到此列表结构中（子类实现）。
     * 非 UFUNCTION，仅供 C++ 内部聚合逻辑使用。
     * @param SourceItems 要聚合的源列表项。
     */
    virtual void AggregateItemsInternal(const TArray<FInstancedStruct>& SourceItems)
    {
        // 基类默认不执行任何操作。子类应重写此方法以实现追加逻辑。
        UE_LOG(LogTemp, Warning, TEXT("AggregateItemsInternal called on FSyListParameterBase itself. No operation performed. Ensure you are using a derived class instance."));
    }

    // 不再需要特定的构造函数
    // FSyListParameterBase() = default;
};

// --- 你可以在这里定义具体的列表类型，继承自 FSyListParameterBase --- 
// 虽然基类已足够通用，但有时为了更清晰的类型区分或添加特定于某类列表的逻辑，
// 你可能还是会定义具体的子类，例如：
/*
USTRUCT(BlueprintType)
struct SYCORE_API FSyStringListParameter : public FSyListParameterBase
{
    GENERATED_BODY()
    // 例如，添加一个验证方法，确保列表只包含 FSyStringValue
    bool ValidateItems() const
    {
        for (const FInstancedStruct& Item : ListItems)
        {
            if (!Item.IsValid() || Item.GetScriptStruct() != FSyStringValue::StaticStruct())
            {
                return false;
            }
        }
        return true;
    }
};
*/

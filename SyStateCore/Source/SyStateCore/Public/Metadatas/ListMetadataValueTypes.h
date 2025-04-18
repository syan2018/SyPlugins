#pragma once

#include "CoreMinimal.h"
#include "Foundation/SyInstancedStruct.h" // 包含基类定义
#include "StructUtils/InstancedStruct.h" // 包含 FInstancedStruct
#include "ListMetadataValueTypes.generated.h"

/**
 * @brief 通用列表参数的基类。
 * 所有需要列表聚合行为（如追加）的参数结构体都应从此类继承。
 * 内部使用 TArray<FInstancedStruct> 来存储列表项，提供最大的灵活性。
 */
USTRUCT(BlueprintType) 
struct SYSTATECORE_API FSyListParameterBase : public FSyBaseInstancedStruct
{
    GENERATED_BODY()

    /** 用于定义子类的对象类型 */
    TObjectPtr<const UScriptStruct> ScriptStruct = nullptr;

    /** 存储列表元素的数组，可以是任何 FInstancedStruct 支持的类型 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="List Value", meta=(DisplayName="List Items"))
    TArray<FInstancedStruct> ListItems;

    // 默认构造函数
    FSyListParameterBase() = default;

    // 可以添加其他方法，例如 AddItem, RemoveItem 等，如果需要的话
    // void AddItem(const FInstancedStruct& NewItem) { ListItems.Add(NewItem); }
};

// --- 你可以在这里定义具体的列表类型，继承自 FSyListParameterBase --- 
// 虽然基类已足够通用，但有时为了更清晰的类型区分或添加特定于某类列表的逻辑，
// 你可能还是会定义具体的子类，例如：
/*
USTRUCT(BlueprintType)
struct SYSTATECORE_API FSyStringListParameter : public FSyListParameterBase
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
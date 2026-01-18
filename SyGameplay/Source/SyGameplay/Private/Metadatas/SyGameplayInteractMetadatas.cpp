#include "Metadatas/SyGameplayInteractMetadatas.h"

// 返回此元数据管理的具体数据类型 FSyInteractionListValue
UScriptStruct* USyInteractionListMetadata::GetValueDataType_Implementation() const
{
	return FSyInteractionListValue::StaticStruct();
}

// 获取存储的值 (FSyInteractionListValue)，将其包装在 FInstancedStruct 中返回
FInstancedStruct USyInteractionListMetadata::GetValueStruct_Implementation() const
{
	FInstancedStruct ValueStruct;
	// 使用 FSyInteractionListValue::StaticStruct() 来初始化 FInstancedStruct
	ValueStruct.InitializeAs<FSyInteractionListValue>(InteractionListValue); 
	return ValueStruct;
}

// 设置存储的值 (FSyInteractionListValue)，从传入的 FInstancedStruct 中提取
void USyInteractionListMetadata::SetValueStruct_Implementation(const FInstancedStruct& InValue)
{
	// 检查传入的 FInstancedStruct 是否有效，并且其类型是否为 FSyInteractionListValue
	if (InValue.IsValid() && InValue.GetScriptStruct() == FSyInteractionListValue::StaticStruct())
	{
		// 尝试获取 FSyInteractionListValue 的指针
		if (const FSyInteractionListValue* ValuePtr = InValue.GetPtr<FSyInteractionListValue>())
		{
			// 将传入的值赋给内部变量
			InteractionListValue = *ValuePtr;
		}
	}
	else
	{
		// 如果类型不匹配或无效，可以记录一个警告
		UE_LOG(LogTemp, Warning, TEXT("USyInteractionListMetadata::SetValueStruct_Implementation: Invalid or mismatched FInstancedStruct provided."));
	}
}

// 获取交互信息列表的只读引用
// 注意：现在通过调用虚函数来获取列表
const TArray<FInstancedStruct>& USyInteractionListMetadata::GetInteractionList() const
{
	// InteractionListValue 是 FSyInteractionListValue 类型，它继承了 FSyListParameterBase
	// 直接调用其重写的虚函数 GetListItemsInternal()
	return InteractionListValue.GetListItemsInternal();
}

// 添加一个新的交互信息到列表中
// 注意：我们直接操作 InteractionListValue 内部的列表
bool USyInteractionListMetadata::AddInteraction(const FInstancedStruct& InteractInfoInstance)
{
	// 验证传入的结构体是否有效且是 FSyInteractInfoBase 的子类
	if (InteractInfoInstance.IsValid() && InteractInfoInstance.GetScriptStruct()->IsChildOf(FSyInteractInfoBase::StaticStruct()))
    {
        // 通过 GetListItemsInternal() 获取可变引用并添加
        InteractionListValue.GetListItemsInternal().Add(InteractInfoInstance);
        return true;
    }
    UE_LOG(LogTemp, Warning, TEXT("USyInteractionListMetadata::AddInteraction: Attempted to add invalid or non-FSyInteractInfoBase struct."));
    return false;
}

// 清空交互信息列表
// 注意：直接操作 InteractionListValue 内部的列表
void USyInteractionListMetadata::ClearInteractions()
{
	// 通过 GetListItemsInternal() 获取可变引用并清空
    InteractionListValue.GetListItemsInternal().Empty();
}

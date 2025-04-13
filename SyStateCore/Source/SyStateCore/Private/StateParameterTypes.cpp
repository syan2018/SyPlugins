#include "StateParameterTypes.h"
#include "DS_TagMetadata.h"
#include "StateMetadataTypes.h"

/**
 * PostSerialize - 序列化后处理
 * 
 * TODO: 重构！！！
 *
 * 没法正常拿到Tag更新回调的下策令人绝望，存在一系列Bug
 * 在Default处要求重复选择Tag才能正常触发PostSerialize逻辑
 * 以及使用LastTag进行标识恶心至极，就就不信没有其它阳间法子了
 * 
 */
void FSyStateParams::PostSerialize(const FArchive& Ar)
{
#if WITH_EDITOR
	
	if (!Ar.IsLoading() && !Ar.IsSaving())
	{
		return;
	}

	if (LastTag == Tag)
	{
		return;
	}
	LastTag = Tag;

	// 如果 Tag 无效，清空参数数组
	if (!Tag.IsValid())
	{
		ClearParams();
		return;
	}

	// 获取 Tag 的元数据
	TArray<UO_TagMetadata*> AllMetadata = UDS_TagMetadata::GetTagMetadata(Tag);
	if (AllMetadata.Num() == 0)
	{
		ClearParams();
		return;
	}

	// 过滤出 StateMetadata
	TArray<USyStateMetadataBase*> StateMetadataList;
	for (UO_TagMetadata* Metadata : AllMetadata)
	{
		if (USyStateMetadataBase* StateMetadata = Cast<USyStateMetadataBase>(Metadata))
		{
			StateMetadataList.Add(StateMetadata);
		}
	}

	// 如果没有 StateMetadata，清空参数数组
	if (StateMetadataList.Num() == 0)
	{
		ClearParams();
		return;
	}

	// 更新参数数组
	TArray<FInstancedStruct> NewParams;
	NewParams.Reserve(StateMetadataList.Num());

	for (USyStateMetadataBase* StateMetadata : StateMetadataList)
	{
		if (StateMetadata && StateMetadata->GetValueDataType())
		{
			FInstancedStruct NewInstance;
			NewInstance.InitializeAs(StateMetadata->GetValueDataType());
			if (NewInstance.IsValid())
			{
				NewParams.Add(NewInstance);
			}
		}
	}

	// 更新参数数组
	Params = MoveTemp(NewParams);

#endif
	
}
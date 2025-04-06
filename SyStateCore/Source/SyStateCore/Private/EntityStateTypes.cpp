// Copyright Epic Games, Inc. All Rights Reserved.

#include "EntityStateTypes.h"
#include "StateMetadataTypes.h"
#include "DS_TagMetadata.h"

void FSyEntityState::ApplyInitData(const FSyEntityInitData& InitData)
{
    // 遍历所有初始化数据
    for (const auto& StatePair : InitData.InitialState)
    {
        const FGameplayTag& StateTag = StatePair.Key;
        const FSyStateParams& StateParams = StatePair.Value;

        // 获取或创建该状态标签的元数据数组
        FSyStateMetadataArray& MetadataArray = StateData.FindOrAdd(StateTag);

        // 应用每个初始化参数
        for (const FSyInstancedStruct& InitParam : StateParams.Params)
        {
            // 从TagMetadata系统获取元数据实例列表
            TArray<UO_TagMetadata*> MetadataList = UDS_TagMetadata::GetTagMetadata(StateTag);
            
            // 遍历所有元数据实例
            for (UO_TagMetadata* Metadata : MetadataList)
            {
                // 初始化元数据对象
                if (USyStateMetadataBase* StateMetadata = Cast<USyStateMetadataBase>(Metadata))
                {
                    StateMetadata->SetStateTag(StateTag);
                    StateMetadata->InitializeFromParams(InitParam);
                    MetadataArray.AddMetadata(StateMetadata);
                }
            }
        }
    }
}

void FSyEntityState::ApplyStateModifications(const TMap<FGameplayTag, FSyStateParams>& StateModifications)
{
    // 遍历所有状态修改
    for (const auto& StatePair : StateModifications)
    {
        const FGameplayTag& StateTag = StatePair.Key;
        const FSyStateParams& StateParams = StatePair.Value;

        // 获取该状态标签的元数据数组
        if (FSyStateMetadataArray* MetadataArray = StateData.Find(StateTag))
        {
            // 应用每个修改参数到所有匹配的元数据对象
            for (const FSyInstancedStruct& ModParam : StateParams.Params)
            {
                // 从TagMetadata系统获取元数据实例列表
                TArray<UO_TagMetadata*> MetadataList = UDS_TagMetadata::GetTagMetadata(StateTag);
                
                // 遍历所有元数据实例
                for (UO_TagMetadata* Metadata : MetadataList)
                {
                    // 查找匹配类型的元数据对象
                    for (TObjectPtr<UO_TagMetadata>& ExistingMetadata : MetadataArray->MetadataArray)
                    {
                        if (USyStateMetadataBase* StateMetadata = Cast<USyStateMetadataBase>(ExistingMetadata))
                        {
                            if (StateMetadata->GetClass() == Metadata->GetClass())
                            {
                                // 应用修改
                                StateMetadata->ApplyModification(ModParam);
                            }
                        }
                    }
                }
            }
        }
    }
}
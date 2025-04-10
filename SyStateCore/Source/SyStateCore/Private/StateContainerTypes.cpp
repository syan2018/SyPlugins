// Copyright Epic Games, Inc. All Rights Reserved.

#include "StateContainerTypes.h"
#include "StateParameterTypes.h" // Include the parameters header
#include "StateMetadataTypes.h" // Include metadata header for USyStateMetadataBase
#include "DS_TagMetadata.h" // Include for UDS_TagMetadata
#include "SyCore/Public/Foundation/Utilities/SyInstancedStruct.h" // Include for FSyInstancedStruct

// Implementation for FSyStateCategories::ApplyInitData
void FSyStateCategories::ApplyInitData(const FSyStateParameterSet& InitData)
{
    // 遍历所有初始化数据
    for (const auto& StatePair : InitData.Parameters) // Use Parameters from FSyStateParameterSet
    {
        const FGameplayTag& StateTag = StatePair.Key;
        const FSyStateParams& StateParams = StatePair.Value;

        // 获取或创建该状态标签的元数据数组
        FSyStateMetadatas& MetadataArray = StateData.FindOrAdd(StateTag);

        // 应用每个初始化参数
        for (const FSyInstancedStruct& InitParam : StateParams.Params)
        {
            // 从TagMetadata系统获取元数据实例列表
            // TODO: Review this logic. Should probably create new instances based on InitParam type?
            // Current logic seems to fetch existing metadata and apply params to them.
            TArray<UO_TagMetadata*> MetadataList = UDS_TagMetadata::GetTagMetadata(StateTag);
            
            // 遍历所有元数据实例
            for (UO_TagMetadata* Metadata : MetadataList)
            {
                // 初始化元数据对象
                if (USyStateMetadataBase* StateMetadata = Cast<USyStateMetadataBase>(Metadata))
                {
                    StateMetadata->SetStateTag(StateTag);
                    StateMetadata->InitializeFromParams(InitParam);
                    MetadataArray.AddMetadata(StateMetadata); // Adds potentially existing metadata again?
                }
            }
        }
    }
}

// Implementation for FSyStateCategories::ApplyStateModifications
void FSyStateCategories::ApplyStateModifications(const TMap<FGameplayTag, FSyStateParams>& StateModifications)
{
    // 遍历所有状态修改
    for (const auto& StatePair : StateModifications)
    {
        const FGameplayTag& StateTag = StatePair.Key;
        const FSyStateParams& StateParams = StatePair.Value;

        // 获取该状态标签的元数据数组
        if (FSyStateMetadatas* MetadataArray = StateData.Find(StateTag))
        {
            // 应用每个修改参数到所有匹配的元数据对象
            for (const FSyInstancedStruct& ModParam : StateParams.Params)
            {
                // 从TagMetadata系统获取元数据实例列表
                // TODO: Review this logic. Should apply ModParam only to relevant metadata instances within MetadataArray.
                TArray<UO_TagMetadata*> MetadataList = UDS_TagMetadata::GetTagMetadata(StateTag);
                
                // 遍历所有元数据实例
                for (UO_TagMetadata* MetadataTemplate : MetadataList) // Use a different name to avoid confusion
                {
                    // 查找匹配类型的元数据对象 in the current StateData
                    for (TObjectPtr<UO_TagMetadata>& ExistingMetadata : MetadataArray->MetadataArray)
                    {
                        if (USyStateMetadataBase* StateMetadata = Cast<USyStateMetadataBase>(ExistingMetadata))
                        {
                            // Check if the existing metadata matches the type from TagMetadata system
                            if (MetadataTemplate && StateMetadata->GetClass() == MetadataTemplate->GetClass())
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
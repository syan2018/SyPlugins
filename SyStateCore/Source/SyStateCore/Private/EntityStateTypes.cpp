// Copyright Epic Games, Inc. All Rights Reserved.

#include "EntityStateTypes.h"

#include "DS_TagMetadata.h"
#include "StateMetadataTypes.h"

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
            // 从TagMetadata系统获取元数据类型
            if (UO_TagMetadata* MetadataTemplate = UDS_TagMetadata::GetTagMetadataByClass(StateTag, USyStateMetadataBase::StaticClass()))
            {
                // 创建新的元数据对象
                if (USyStateMetadataBase* StateMetadata = Cast<USyStateMetadataBase>(
                    NewObject<UO_TagMetadata>(GetTransientPackage(), MetadataTemplate->GetClass())))
                {
                    // 初始化元数据对象
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
                // 从TagMetadata系统获取元数据类型
                if (UO_TagMetadata* MetadataTemplate = UDS_TagMetadata::GetTagMetadataByClass(StateTag, USyStateMetadataBase::StaticClass()))
                {
                    // 查找匹配类型的元数据对象
                    for (TObjectPtr<UO_TagMetadata>& Metadata : MetadataArray->MetadataArray)
                    {
                        if (USyStateMetadataBase* StateMetadata = Cast<USyStateMetadataBase>(Metadata))
                        {
                            if (StateMetadata->GetClass() == MetadataTemplate->GetClass())
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
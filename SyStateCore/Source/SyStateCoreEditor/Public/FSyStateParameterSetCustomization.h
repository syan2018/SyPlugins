// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyHandle.h"
#include "Widgets/Text/STextBlock.h"
#include "StateMetadataTypes.h" // For USyStateMetadataBase

class FSyStateParameterSetCustomization : public IPropertyTypeCustomization
{
public:
    static TSharedRef<IPropertyTypeCustomization> MakeInstance();

    /** IPropertyTypeCustomization interface */
    virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
    virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
    // Handles customization for a single Tag -> Params pair in the TMap
    void CustomizeMapElement(TSharedRef<IPropertyHandle> MapPropertyHandle, TSharedRef<IPropertyHandle> KeyHandle, TSharedRef<IPropertyHandle> ValueHandle);

    // Updates the FSyStateParams struct based on the selected tag
    static void UpdateParameterStructForTag(const FGameplayTag& Tag, const TSharedRef<IPropertyHandle>& ParamsPropertyHandle);

    // 序列化参数结构体
    static void SerializeParameterStruct(const FInstancedStruct& Instance, const TSharedRef<IPropertyHandle>& Handle);

    // 从元数据创建参数结构体
    static FInstancedStruct CreateParameterStructFromMetadata(const USyStateMetadataBase* StateMetadata);

    /** Utility for property view customization */
    TWeakPtr<IPropertyUtilities> PropertyUtilities;

    TSharedPtr<IPropertyHandle> StructHandle;
    uint32 PreviousNumElements = 0;
    uint32 CurrentNumElements = 0;

    // 存储当前正在处理的标签
    FGameplayTag CurrentTag;
}; 
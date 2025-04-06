// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "OperationTypes.h"

/**
 * FSyOperationDetailsCustomization - 操作详情自定义类
 * 
 * 用于自定义FSyOperation在编辑器中的显示和编辑体验
 */
class SYOPERATION_API FSyOperationDetailsCustomization : public IDetailCustomization
{
public:
    /** 创建自定义实例 */
    static TSharedRef<IDetailCustomization> MakeInstance();

    /** IDetailCustomization接口 */
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
    /** 处理Tag变更 */
    void OnSourceTagChanged();
    void OnModifierTagChanged();
    void OnTargetTagChanged();

    /** 获取当前编辑的对象 */
    FSyOperation* GetOperationBeingEdited() const;

    /** 当前编辑的对象 */
    TWeakObjectPtr<UObject> EditedObject;

    /** 属性句柄 */
    TSharedPtr<IPropertyHandle> SourceTypeTagHandle;
    TSharedPtr<IPropertyHandle> ModifierTagHandle;
    TSharedPtr<IPropertyHandle> TargetTypeTagHandle;

    /** DetailBuilder引用 */
    IDetailLayoutBuilder* DetailBuilderPtr;
}; 
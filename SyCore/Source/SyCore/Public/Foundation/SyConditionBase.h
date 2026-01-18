#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SyConditionBase.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType, AutoExpandCategories = ("Default"), EditInlineNew,
	meta=(ContextMenuCategory = "SyPlugins", ContextMenuEntryName = "SyPlugins|Condition", ContextMenuPrefix = "CDT_"))
class SYCORE_API UOSyConditionBase : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(Category = "SyPlugins|Condition", BlueprintCallable, BlueprintPure, BlueprintNativeEvent)
	bool IsConditionMet();

	virtual class UWorld* GetWorld() const override;
};

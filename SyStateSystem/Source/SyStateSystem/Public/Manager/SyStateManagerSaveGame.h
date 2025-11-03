// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Manager/StateModificationRecord.h" // 包含记录的定义
#include "SyStateManagerSaveGame.generated.h"

/**
 * 用于保存 SyStateManager 操作日志的存档对象。
 */
UCLASS()
class SYSTATESYSTEM_API USyStateManagerSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    /** 需要保存的操作日志 */
    UPROPERTY(VisibleAnywhere, Category = Basic)
    TArray<FSyStateModificationRecord> SavedModificationLog;

    /** 存档标识符 (可选，用于版本控制或识别) */
    UPROPERTY(VisibleAnywhere, Category = Basic)
    FString SaveGameVersion = TEXT("1.0");
}; 
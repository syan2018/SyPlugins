#pragma once

#include "CoreMinimal.h"

/**
 * 子系统工具类
 * 提供通用的子系统获取方法
 */
namespace SySubsystemUtils
{
    /**
     * 获取指定类型的子系统
     * @param World - 世界对象
     * @return 子系统实例指针，如果获取失败则返回nullptr
     */
    template<typename T>
    static T* GetSubsystem(const UWorld* World)
    {
        if (World && World->GetGameInstance())
        {
            return World->GetGameInstance()->GetSubsystem<T>();
        }
        return nullptr;
    }
}

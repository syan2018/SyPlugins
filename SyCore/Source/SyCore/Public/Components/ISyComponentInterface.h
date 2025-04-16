#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ISyComponentInterface.generated.h"

/**
 * ISyComponentInterface - Sy系列组件的统一接口
 * 用于标识所有属于Sy系列的功能组件，提供统一的类型判断方式
 */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class USyComponentInterface : public UInterface
{
    GENERATED_BODY()
};

class SYCORE_API ISyComponentInterface
{
    GENERATED_BODY()

public:
    /**
     * @brief 获取组件的类型标识
     * @return 返回组件的类型标识字符串
     */
    virtual FName GetComponentType() const = 0;
}; 
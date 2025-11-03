#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ISyComponentInterface.generated.h"

/**
 * @brief Sy 组件初始化阶段
 * 定义组件的初始化顺序，数值越小越先初始化
 */
UENUM(BlueprintType)
enum class ESyComponentInitPhase : uint8
{
    /** 核心数据层：StateComponent 等核心数据管理组件 */
    Core = 0        UMETA(DisplayName = "Core"),
    
    /** 数据管理层：Inventory, Quest 等数据管理组件 */
    DataManagement = 10     UMETA(DisplayName = "Data Management"),
    
    /** 逻辑层：AI, Behavior 等逻辑组件 */
    Logic = 50      UMETA(DisplayName = "Logic"),
    
    /** 功能层：Interaction, Spawn 等功能组件 */
    Functional = 100        UMETA(DisplayName = "Functional"),
    
    /** 表现层：UI, VFX 等表现组件 */
    Presentation = 200      UMETA(DisplayName = "Presentation")
};

/**
 * ISyComponentInterface - Sy系列组件的统一接口
 * 用于标识所有属于Sy系列的功能组件，提供统一的类型判断方式和生命周期管理
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

    /**
     * @brief 获取组件的初始化阶段
     * @return 初始化阶段，阶段值越小越先初始化
     */
    virtual ESyComponentInitPhase GetInitializationPhase() const 
    { 
        return ESyComponentInitPhase::Functional; 
    }

    /**
     * @brief 由 EntityComponent 协调调用的初始化方法
     * @details 此方法在 BeginPlay 之后，按初始化阶段顺序调用
     *          确保组件可以安全地访问依赖的其他组件
     * @note 子类应该重写此方法来实现自己的初始化逻辑
     */
    virtual void OnSyComponentInitialized() {}
}; 
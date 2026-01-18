#pragma once

#include "CoreMinimal.h"
#include "Components/Base/SpawnComponent.h"
#include "Foundation/ISyComponentInterface.h"
#include "SySpawnComponent.generated.h"

/**
 * SySpawnComponent - 实体生成组件
 * 职责：
 * 1. 继承自SpawnComponent，提供基础的生成功能
 * 2. 监听状态变化，根据"State.Spawner.Enable"标签控制生成能力
 * 3. 实现ISyComponentInterface接口，支持Sy系列组件的统一管理
 */
UCLASS(ClassGroup=(SyGameplay), meta=(BlueprintSpawnableComponent))
class SYGAMEPLAY_API USySpawnComponent : public USpawnComponent, public ISyComponentInterface
{
    GENERATED_BODY()

public:
    USySpawnComponent();

    // 实现ISyComponentInterface接口
    virtual FName GetComponentType() const override { return TEXT("Spawn"); }
    
    // SpawnComponent 在功能阶段初始化
    virtual ESyComponentInitPhase GetInitializationPhase() const override 
    { 
        return ESyComponentInitPhase::Functional; 
    }
    
    // 由 EntityComponent 调用的初始化方法
    virtual void OnSyComponentInitialized() override;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    /** 缓存关联的StateComponent指针 */
    UPROPERTY(Transient)
    TObjectPtr<class USyStateComponent> StateComponent;

    /** 处理状态变化 */
    UFUNCTION()
    void HandleStateChanged();

    /** 查找并缓存StateComponent */
    void FindAndCacheStateComponent();
};

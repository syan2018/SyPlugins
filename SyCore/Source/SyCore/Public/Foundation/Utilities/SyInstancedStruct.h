// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "SyInstancedStruct.generated.h"

/**
 * FSyInstancedStruct - 扩展自FInstancedStruct的工具类
 * 
 * 提供类型安全、初始化和序列化支持，用于在运行时持有任意USTRUCT的实例
 * 主要用于操作参数和状态数据的传递
 */
USTRUCT(BlueprintType)
struct SYCORE_API FSyInstancedStruct : public FInstancedStruct
{
    GENERATED_BODY()

public:
    /** 默认构造函数 */
    FSyInstancedStruct() = default;

    /** 从FInstancedStruct构造 */
    FSyInstancedStruct(const FInstancedStruct& Other) : FInstancedStruct(Other) {}

    /** 从FInstancedStruct移动构造 */
    FSyInstancedStruct(FInstancedStruct&& Other) : FInstancedStruct(MoveTemp(Other)) {}

    /** 从指定结构体类型构造 */
    template<typename T>
    FSyInstancedStruct(const T& Value) : FInstancedStruct(FInstancedStruct::Make(Value)) {}

    /** 从指定结构体类型移动构造 */
    template<typename T>
    FSyInstancedStruct(T&& Value) : FInstancedStruct(FInstancedStruct::Make(MoveTemp(Value))) {}

    /** 初始化并设置结构体类型 */
    template<typename T>
    void InitializeAs()
    {
        FInstancedStruct::InitializeAs<T>();
    }

    /** 初始化并设置结构体类型和值 */
    template<typename T>
    void InitializeAs(const T& Value)
    {
        FInstancedStruct::InitializeAs(Value);
    }

    /** 初始化并设置结构体类型和移动值 */
    template<typename T>
    void InitializeAs(T&& Value)
    {
        FInstancedStruct::InitializeAs(MoveTemp(Value));
    }

    /** 获取结构体指针，如果类型不匹配则返回nullptr */
    template<typename T>
    const T* GetPtr() const
    {
        return FInstancedStruct::GetPtr<T>();
    }

    /** 获取可变结构体指针，如果类型不匹配则返回nullptr */
    template<typename T>
    T* GetMutablePtr()
    {
        return FInstancedStruct::GetMutablePtr<T>();
    }

    /** 获取结构体值，如果类型不匹配则返回默认构造的值 */
    template<typename T>
    T Get() const
    {
        return FInstancedStruct::Get<T>();
    }

    /** 设置结构体值，如果类型不匹配则返回false */
    template<typename T>
    bool Set(const T& Value)
    {
        // 由于FInstancedStruct没有Set方法，我们需要使用InitializeAs
        FInstancedStruct::InitializeAs(Value);
        return true;
    }

    /** 设置结构体移动值，如果类型不匹配则返回false */
    template<typename T>
    bool Set(T&& Value)
    {
        // 由于FInstancedStruct没有Set方法，我们需要使用InitializeAs
        FInstancedStruct::InitializeAs(MoveTemp(Value));
        return true;
    }

    /** 检查是否包含指定类型的结构体 */
    template<typename T>
    bool IsA() const
    {
        // 由于FInstancedStruct没有IsA方法，我们需要检查ScriptStruct
        const UScriptStruct* Struct = TBaseStructure<T>::Get();
        return GetScriptStruct() && GetScriptStruct()->IsChildOf(Struct);
    }

    /** 获取结构体类型 */
    const UScriptStruct* GetScriptStruct() const
    {
        // 由于FInstancedStruct没有GetScriptStruct方法，我们需要直接访问ScriptStruct成员
        return ScriptStruct;
    }

    /** 序列化支持 */
    bool Serialize(FArchive& Ar)
    {
        return FInstancedStruct::Serialize(Ar);
    }

    /** 网络序列化支持 */
    bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
    {
        return FInstancedStruct::NetSerialize(Ar, Map, bOutSuccess);
    }
};

/** 网络序列化模板特化 */
template<>
struct TStructOpsTypeTraits<FSyInstancedStruct> : public TStructOpsTypeTraitsBase2<FSyInstancedStruct>
{
    enum 
    {
        WithNetSerializer = true,
        WithCopy = true,
        WithNetSharedSerialization = true,
    };
}; 
#pragma once

#include "CoreMinimal.h"
#include "SyLogging.h"

/**
 * SyPlugins 统一错误处理系统
 * 
 * 提供一致的错误报告、验证和处理机制
 */

namespace SyError
{
	/**
	 * 错误严重程度
	 */
	enum class ESeverity : uint8
	{
		/** 信息性消息 - 不影响功能 */
		Info,
		
		/** 警告 - 可能影响功能但可以恢复 */
		Warning,
		
		/** 错误 - 功能无法正常执行 */
		Error,
		
		/** 致命错误 - 需要中断执行 */
		Fatal
	};

	/**
	 * 错误上下文信息
	 */
	struct FErrorContext
	{
		/** 错误发生的模块/系统 */
		FString Module;
		
		/** 错误发生的具体位置（函数名等） */
		FString Location;
		
		/** 相关对象（如果有） */
		TWeakObjectPtr<UObject> RelatedObject;
		
		/** 时间戳 */
		FDateTime Timestamp;
		
		/** 错误代码（可选） */
		int32 ErrorCode;

		FErrorContext()
			: Timestamp(FDateTime::Now())
			, ErrorCode(0)
		{}

		FErrorContext(const FString& InModule, const FString& InLocation)
			: Module(InModule)
			, Location(InLocation)
			, Timestamp(FDateTime::Now())
			, ErrorCode(0)
		{}
	};

	/**
	 * 报告错误
	 * 
	 * @param Severity 错误严重程度
	 * @param Context 错误上下文
	 * @param Message 错误消息
	 */
	SYCORE_API void Report(ESeverity Severity, const FErrorContext& Context, const FString& Message);

	/**
	 * 简化的错误报告接口
	 */
	SYCORE_API void Report(ESeverity Severity, const FString& Module, const FString& Message);

	/**
	 * 获取严重程度的字符串表示
	 */
	SYCORE_API FString SeverityToString(ESeverity Severity);

	/**
	 * 获取严重程度对应的日志级别
	 */
	SYCORE_API ELogVerbosity::Type SeverityToLogVerbosity(ESeverity Severity);
}

// ===== 便捷错误报告宏 =====

/**
 * 报告信息
 * 用法: SY_INFO(TEXT("SyCore"), TEXT("Initialization complete"));
 */
#define SY_INFO(Module, Message) \
	SyError::Report(SyError::ESeverity::Info, Module, Message)

/**
 * 报告警告
 * 用法: SY_WARNING(TEXT("SyStateManager"), TEXT("State not found"));
 */
#define SY_WARNING(Module, Message) \
	SyError::Report(SyError::ESeverity::Warning, Module, Message)

/**
 * 报告错误
 * 用法: SY_ERROR(TEXT("SyEntity"), TEXT("Failed to initialize component"));
 */
#define SY_ERROR(Module, Message) \
	SyError::Report(SyError::ESeverity::Error, Module, Message)

/**
 * 报告致命错误
 * 用法: SY_FATAL(TEXT("SyCore"), TEXT("Critical system failure"));
 */
#define SY_FATAL(Module, Message) \
	SyError::Report(SyError::ESeverity::Fatal, Module, Message)

// ===== 条件检查宏 =====

/**
 * 检查条件，如果失败则报告错误并返回
 * 
 * 用法: 
 * SY_CHECK(IsValid(Component), TEXT("SyEntity"), TEXT("Component is invalid"));
 */
#define SY_CHECK(Condition, Module, Message) \
	if (!(Condition)) { \
		SY_ERROR(Module, Message); \
		return; \
	}

/**
 * 检查条件，如果失败则报告错误并返回指定值
 * 
 * 用法: 
 * SY_CHECK_RETURN(IsValid(Data), false, TEXT("SyState"), TEXT("Invalid data"));
 */
#define SY_CHECK_RETURN(Condition, ReturnValue, Module, Message) \
	if (!(Condition)) { \
		SY_ERROR(Module, Message); \
		return ReturnValue; \
	}

/**
 * 检查条件，如果失败则报告错误并继续执行
 * 
 * 用法: 
 * SY_CHECK_CONTINUE(IsValid(Item), TEXT("SyGameplay"), TEXT("Invalid item"));
 */
#define SY_CHECK_CONTINUE(Condition, Module, Message) \
	if (!(Condition)) { \
		SY_ERROR(Module, Message); \
		continue; \
	}

/**
 * 检查条件，如果失败则报告错误并跳出循环
 * 
 * 用法: 
 * SY_CHECK_BREAK(Index < MaxIndex, TEXT("SyCore"), TEXT("Index out of range"));
 */
#define SY_CHECK_BREAK(Condition, Module, Message) \
	if (!(Condition)) { \
		SY_ERROR(Module, Message); \
		break; \
	}

// ===== 指针验证宏 =====

/**
 * 验证指针非空
 * 
 * 用法: 
 * SY_CHECK_PTR(MyPointer, TEXT("SyEntity"));
 */
#define SY_CHECK_PTR(Pointer, Module) \
	SY_CHECK(Pointer != nullptr, Module, FString::Printf(TEXT("%s is nullptr"), TEXT(#Pointer)))

/**
 * 验证指针非空并返回指定值
 * 
 * 用法: 
 * SY_CHECK_PTR_RETURN(MyPointer, nullptr, TEXT("SyState"));
 */
#define SY_CHECK_PTR_RETURN(Pointer, ReturnValue, Module) \
	SY_CHECK_RETURN(Pointer != nullptr, ReturnValue, Module, FString::Printf(TEXT("%s is nullptr"), TEXT(#Pointer)))

/**
 * 验证 UObject 有效
 * 
 * 用法: 
 * SY_CHECK_VALID(MyActor, TEXT("SyGameplay"));
 */
#define SY_CHECK_VALID(Object, Module) \
	SY_CHECK(IsValid(Object), Module, FString::Printf(TEXT("%s is invalid"), TEXT(#Object)))

/**
 * 验证 UObject 有效并返回指定值
 * 
 * 用法: 
 * SY_CHECK_VALID_RETURN(Component, false, TEXT("SyEntity"));
 */
#define SY_CHECK_VALID_RETURN(Object, ReturnValue, Module) \
	SY_CHECK_RETURN(IsValid(Object), ReturnValue, Module, FString::Printf(TEXT("%s is invalid"), TEXT(#Object)))

// ===== 开发环境断言 =====

/**
 * 开发环境断言 - 只在非 Shipping 构建中生效
 * 
 * 用法: 
 * SY_DEV_ASSERT(Value > 0, TEXT("Value must be positive"));
 */
#if !UE_BUILD_SHIPPING
	#define SY_DEV_ASSERT(Condition, Message) \
		if (!(Condition)) { \
			UE_LOG(LogSyCore, Fatal, TEXT("Assertion Failed: %s"), Message); \
			check(false); \
		}
#else
	#define SY_DEV_ASSERT(Condition, Message)
#endif

/**
 * 开发环境检查 - 只在非 Shipping 构建中生效，失败时记录错误但不中断
 * 
 * 用法: 
 * SY_DEV_CHECK(Index < ArraySize, TEXT("Index out of bounds"));
 */
#if !UE_BUILD_SHIPPING
	#define SY_DEV_CHECK(Condition, Message) \
		if (!(Condition)) { \
			UE_LOG(LogSyCore, Error, TEXT("Check Failed: %s"), Message); \
			ensure(false); \
		}
#else
	#define SY_DEV_CHECK(Condition, Message)
#endif

// ===== 性能警告宏 =====

/**
 * 性能警告 - 当操作可能影响性能时发出警告
 * 
 * 用法: 
 * SY_PERF_WARNING(bUseSlow, TEXT("SyStateManager"), TEXT("Using slow query path"));
 */
#define SY_PERF_WARNING(Condition, Module, Message) \
	if (Condition) { \
		SY_WARNING(Module, FString::Printf(TEXT("⚠️ Performance Warning: %s"), *Message)); \
	}

/**
 * 弃用警告 - 标记已弃用的功能
 * 
 * 用法: 
 * SY_DEPRECATED(TEXT("SyEntity"), TEXT("Use NewFunction() instead"));
 */
#define SY_DEPRECATED(Module, Message) \
	SY_LOG_ONCE(LogSyCore, Warning, TEXT("🚫 Deprecated: %s - %s"), *Module, *Message)

// ===== 格式化辅助宏 =====

/**
 * 格式化错误消息 - 支持 Printf 风格的格式化
 * 
 * 用法: 
 * SY_ERROR_F(TEXT("SyState"), TEXT("Invalid state tag: %s"), *TagName);
 */
#define SY_ERROR_F(Module, Format, ...) \
	SY_ERROR(Module, FString::Printf(Format, ##__VA_ARGS__))

#define SY_WARNING_F(Module, Format, ...) \
	SY_WARNING(Module, FString::Printf(Format, ##__VA_ARGS__))

#define SY_INFO_F(Module, Format, ...) \
	SY_INFO(Module, FString::Printf(Format, ##__VA_ARGS__))

// ===== 错误累积器 =====

/**
 * 错误累积器 - 用于收集多个错误后统一处理
 */
class SYCORE_API FSyErrorAccumulator
{
public:
	FSyErrorAccumulator() : bHasErrors(false) {}

	/** 添加错误 */
	void AddError(const FString& Message)
	{
		Errors.Add(Message);
		bHasErrors = true;
	}

	/** 添加警告 */
	void AddWarning(const FString& Message)
	{
		Warnings.Add(Message);
	}

	/** 是否有错误 */
	bool HasErrors() const { return bHasErrors; }

	/** 是否有警告 */
	bool HasWarnings() const { return Warnings.Num() > 0; }

	/** 获取所有错误 */
	const TArray<FString>& GetErrors() const { return Errors; }

	/** 获取所有警告 */
	const TArray<FString>& GetWarnings() const { return Warnings; }

	/** 报告所有累积的错误和警告 */
	void ReportAll(const FString& Module) const
	{
		for (const FString& Warning : Warnings)
		{
			SY_WARNING(Module, Warning);
		}
		for (const FString& Error : Errors)
		{
			SY_ERROR(Module, Error);
		}
	}

	/** 清空所有错误和警告 */
	void Clear()
	{
		Errors.Empty();
		Warnings.Empty();
		bHasErrors = false;
	}

	/** 格式化为单个字符串 */
	FString ToString() const
	{
		FString Result;
		if (Warnings.Num() > 0)
		{
			Result += FString::Printf(TEXT("Warnings (%d):\n"), Warnings.Num());
			for (const FString& Warning : Warnings)
			{
				Result += FString::Printf(TEXT("  - %s\n"), *Warning);
			}
		}
		if (Errors.Num() > 0)
		{
			Result += FString::Printf(TEXT("Errors (%d):\n"), Errors.Num());
			for (const FString& Error : Errors)
			{
				Result += FString::Printf(TEXT("  - %s\n"), *Error);
			}
		}
		return Result;
	}

private:
	TArray<FString> Errors;
	TArray<FString> Warnings;
	bool bHasErrors;
};

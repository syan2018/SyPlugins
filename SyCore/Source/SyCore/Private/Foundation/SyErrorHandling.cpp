#include "Foundation/SyErrorHandling.h"
#include "Foundation/SyLogging.h"

namespace SyError
{
	void Report(ESeverity Severity, const FErrorContext& Context, const FString& Message)
	{
		// 构建完整的错误消息
		FString FullMessage = Message;
		
		if (!Context.Module.IsEmpty())
		{
			FullMessage = FString::Printf(TEXT("[%s] %s"), *Context.Module, *Message);
		}
		
		if (!Context.Location.IsEmpty())
		{
			FullMessage += FString::Printf(TEXT(" (Location: %s)"), *Context.Location);
		}
		
		if (Context.RelatedObject.IsValid())
		{
			FullMessage += FString::Printf(TEXT(" (Object: %s)"), *Context.RelatedObject->GetName());
		}
		
		if (Context.ErrorCode != 0)
		{
			FullMessage += FString::Printf(TEXT(" [Code: %d]"), Context.ErrorCode);
		}
		
		// 根据严重程度选择日志输出
		ELogVerbosity::Type LogVerbosity = SeverityToLogVerbosity(Severity);
		
		switch (Severity)
		{
		case ESeverity::Info:
			UE_LOG(LogSyCore, Log, TEXT("ℹ️ INFO: %s"), *FullMessage);
			break;
			
		case ESeverity::Warning:
			UE_LOG(LogSyCore, Warning, TEXT("⚠️ WARNING: %s"), *FullMessage);
			break;
			
		case ESeverity::Error:
			UE_LOG(LogSyCore, Error, TEXT("❌ ERROR: %s"), *FullMessage);
			// 在开发环境中，错误会触发 ensure
			#if !UE_BUILD_SHIPPING
			ensure(false);
			#endif
			break;
			
		case ESeverity::Fatal:
			UE_LOG(LogSyCore, Fatal, TEXT("💀 FATAL: %s"), *FullMessage);
			// Fatal 错误会中断执行
			check(false);
			break;
		}
		
		// TODO: 可以在这里添加错误报告到远程服务器或错误收集系统
	}

	void Report(ESeverity Severity, const FString& Module, const FString& Message)
	{
		FErrorContext Context;
		Context.Module = Module;
		Report(Severity, Context, Message);
	}

	FString SeverityToString(ESeverity Severity)
	{
		switch (Severity)
		{
		case ESeverity::Info:
			return TEXT("Info");
		case ESeverity::Warning:
			return TEXT("Warning");
		case ESeverity::Error:
			return TEXT("Error");
		case ESeverity::Fatal:
			return TEXT("Fatal");
		default:
			return TEXT("Unknown");
		}
	}

	ELogVerbosity::Type SeverityToLogVerbosity(ESeverity Severity)
	{
		switch (Severity)
		{
		case ESeverity::Info:
			return ELogVerbosity::Log;
		case ESeverity::Warning:
			return ELogVerbosity::Warning;
		case ESeverity::Error:
			return ELogVerbosity::Error;
		case ESeverity::Fatal:
			return ELogVerbosity::Fatal;
		default:
			return ELogVerbosity::Warning;
		}
	}
}

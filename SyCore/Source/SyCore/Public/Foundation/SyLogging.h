#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

/**
 * SyPlugins 统一日志系统
 * 
 * 为每个模块定义专用的日志类别，便于过滤和调试
 * 使用统一的日志宏来保持一致的日志格式
 */

// ===== 日志类别定义 =====

/** SyCore 模块日志 - 基础设施、标识、消息总线 */
DECLARE_LOG_CATEGORY_EXTERN(LogSyCore, Log, All);

/** SyStateCore 模块日志 - 状态数据结构 */
DECLARE_LOG_CATEGORY_EXTERN(LogSyStateCore, Log, All);

/** SyStateManager 模块日志 - 状态管理和记录 */
DECLARE_LOG_CATEGORY_EXTERN(LogSyStateManager, Log, All);

/** SyOperation 模块日志 - 操作定义 */
DECLARE_LOG_CATEGORY_EXTERN(LogSyOperation, Log, All);

/** SyEntity 模块日志 - 实体框架 */
DECLARE_LOG_CATEGORY_EXTERN(LogSyEntity, Log, All);

/** SyMessage 消息系统日志 - 消息总线和通信 */
DECLARE_LOG_CATEGORY_EXTERN(LogSyMessage, Log, All);

/** SyFlowImpl 模块日志 - Flow 集成 */
DECLARE_LOG_CATEGORY_EXTERN(LogSyFlowImpl, Log, All);

/** SyGameplay 模块日志 - 游戏玩法 */
DECLARE_LOG_CATEGORY_EXTERN(LogSyGameplay, Log, All);


// ===== 统一日志宏 =====

/**
 * 基础日志宏 - 自动添加文件名、行号和函数名
 * 
 * 用法: SY_LOG(LogSyCore, Warning, TEXT("Something happened: %s"), *SomeString);
 */
#define SY_LOG(Category, Verbosity, Format, ...) \
    UE_LOG(Category, Verbosity, TEXT("[%s:%d] %s: ") Format, \
        TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), ##__VA_ARGS__)

/**
 * 函数入口日志 - 记录函数被调用
 * 
 * 用法: SY_LOG_FUNC();
 */
#define SY_LOG_FUNC(Category) \
    UE_LOG(Category, VeryVerbose, TEXT("[%s:%d] %s: Function entered"), \
        TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__))

/**
 * 函数入口日志（带参数）- 记录函数被调用及参数
 * 
 * 用法: SY_LOG_FUNC_PARAMS(LogSyCore, TEXT("bForceReload=%d"), bForceReload);
 */
#define SY_LOG_FUNC_PARAMS(Category, Format, ...) \
    UE_LOG(Category, VeryVerbose, TEXT("[%s:%d] %s: ") Format, \
        TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), ##__VA_ARGS__)

/**
 * 条件日志 - 只在条件为真时记录
 * 
 * 用法: SY_CLOG(bDebugMode, LogSyCore, Log, TEXT("Debug info: %d"), Value);
 */
#define SY_CLOG(Condition, Category, Verbosity, Format, ...) \
    UE_CLOG(Condition, Category, Verbosity, TEXT("[%s:%d] %s: ") Format, \
        TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), ##__VA_ARGS__)


// ===== 性能追踪宏 =====

/**
 * 作用域性能追踪
 * 在作用域开始时记录，结束时自动记录耗时
 * 
 * 用法: 
 * {
 *     SY_SCOPED_PERF(LogSyCore, TEXT("MyFunction"));
 *     // ... 代码 ...
 * }
 */
#if !UE_BUILD_SHIPPING
    #define SY_SCOPED_PERF(Category, ScopeName) \
        FScopedDurationTimer ANONYMOUS_VARIABLE(SyPerfTimer)( \
            [](double DurationMs) { \
                UE_LOG(Category, Verbose, TEXT("⏱️ Performance: %s took %.2f ms"), ScopeName, DurationMs); \
            } \
        )
#else
    #define SY_SCOPED_PERF(Category, ScopeName)
#endif


// ===== 调试辅助宏 =====

/**
 * 一次性日志 - 该日志在程序运行期间只会输出一次
 * 
 * 用法: SY_LOG_ONCE(LogSyCore, Warning, TEXT("This will only show once"));
 */
#define SY_LOG_ONCE(Category, Verbosity, Format, ...) \
    do { \
        static bool bLogged_##__LINE__ = false; \
        if (!bLogged_##__LINE__) { \
            UE_LOG(Category, Verbosity, TEXT("[%s:%d] %s: ") Format, \
                TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), ##__VA_ARGS__); \
            bLogged_##__LINE__ = true; \
        } \
    } while (0)

/**
 * 频率限制日志 - 限制日志输出频率（每N秒最多输出一次）
 * 
 * 用法: SY_LOG_THROTTLE(5.0f, LogSyCore, Warning, TEXT("High frequency event"));
 */
#define SY_LOG_THROTTLE(Seconds, Category, Verbosity, Format, ...) \
    do { \
        static double LastLogTime_##__LINE__ = -1000.0; \
        double CurrentTime = FPlatformTime::Seconds(); \
        if (CurrentTime - LastLogTime_##__LINE__ >= Seconds) { \
            UE_LOG(Category, Verbosity, TEXT("[%s:%d] %s: ") Format, \
                TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), ##__VA_ARGS__); \
            LastLogTime_##__LINE__ = CurrentTime; \
        } \
    } while (0)


// ===== 对象日志辅助 =====

/**
 * 记录 UObject 信息
 * 
 * 用法: SY_LOG_OBJECT(LogSyCore, Log, MyActor, TEXT("Actor spawned"));
 */
#define SY_LOG_OBJECT(Category, Verbosity, Object, Format, ...) \
    UE_LOG(Category, Verbosity, TEXT("[%s:%d] %s [Object:%s]: ") Format, \
        TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), \
        Object ? *Object->GetName() : TEXT("nullptr"), ##__VA_ARGS__)

/**
 * 记录 Actor 位置信息
 * 
 * 用法: SY_LOG_ACTOR_LOCATION(LogSyGameplay, Log, MyActor, TEXT("Actor moved"));
 */
#define SY_LOG_ACTOR_LOCATION(Category, Verbosity, Actor, Format, ...) \
    do { \
        if (Actor) { \
            FVector Loc = Actor->GetActorLocation(); \
            UE_LOG(Category, Verbosity, TEXT("[%s:%d] %s [Actor:%s @ (%.1f,%.1f,%.1f)]: ") Format, \
                TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), \
                *Actor->GetName(), Loc.X, Loc.Y, Loc.Z, ##__VA_ARGS__); \
        } else { \
            UE_LOG(Category, Verbosity, TEXT("[%s:%d] %s [Actor:nullptr]: ") Format, \
                TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), ##__VA_ARGS__); \
        } \
    } while (0)


// ===== 编译时日志控制 =====

/**
 * 开发版本日志 - 只在 Development 和 Debug 构建中输出
 */
#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
    #define SY_DEV_LOG(Category, Verbosity, Format, ...) \
        SY_LOG(Category, Verbosity, Format, ##__VA_ARGS__)
#else
    #define SY_DEV_LOG(Category, Verbosity, Format, ...)
#endif

/**
 * 调试版本日志 - 只在 Debug 构建中输出
 */
#if UE_BUILD_DEBUG
    #define SY_DEBUG_LOG(Category, Verbosity, Format, ...) \
        SY_LOG(Category, Verbosity, Format, ##__VA_ARGS__)
#else
    #define SY_DEBUG_LOG(Category, Verbosity, Format, ...)
#endif


// ===== GameplayTag 日志辅助 =====

/**
 * 记录 GameplayTag 信息
 * 
 * 用法: SY_LOG_TAG(LogSyState, Log, StateTag, TEXT("State changed"));
 */
#define SY_LOG_TAG(Category, Verbosity, Tag, Format, ...) \
    UE_LOG(Category, Verbosity, TEXT("[%s:%d] %s [Tag:%s]: ") Format, \
        TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), \
        *Tag.ToString(), ##__VA_ARGS__)

/**
 * 记录 GameplayTagContainer 信息
 * 
 * 用法: SY_LOG_TAGS(LogSyEntity, Log, EntityTags, TEXT("Entity tags updated"));
 */
#define SY_LOG_TAGS(Category, Verbosity, TagContainer, Format, ...) \
    UE_LOG(Category, Verbosity, TEXT("[%s:%d] %s [Tags:%s]: ") Format, \
        TEXT(__FILE__), __LINE__, TEXT(__FUNCTION__), \
        *TagContainer.ToStringSimple(), ##__VA_ARGS__)


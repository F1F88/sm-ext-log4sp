#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/dist_sink.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"

#include "log4sp/utils.h"
#include "log4sp/sink_register.h"
#include "log4sp/logger_register.h"
#include "log4sp/adapter/base_sink.h"
#include "log4sp/adapter/sync_logger.h"
#include "log4sp/adapter/async_logger.h"


/**
 * public native Logger(const char[] name, Sink[] sinks, int numSinks, bool async = false, AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block);
 */
static cell_t Logger(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_register::instance().get(name) != nullptr)
    {
        ctx->ReportError("Logger with name '%s' already exists.", name);
        return BAD_HANDLE;
    }

    cell_t *sinks;
    ctx->LocalToPhysAddr(params[2], &sinks);

    auto numSinks = static_cast<unsigned int>(params[3]);

    std::vector<spdlog::sink_ptr> sinksList;
    sinksList.reserve(numSinks);

    for (unsigned int i = 0; i < numSinks; ++i)
    {
        auto sinkHandle     = static_cast<Handle_t>(sinks[i]);
        auto sinkAdapterRaw = log4sp::base_sink::read(sinkHandle, ctx);
        if (sinkAdapterRaw == nullptr)
        {
            return BAD_HANDLE;
        }

        sinksList.push_back(sinkAdapterRaw->raw());
    }

    std::shared_ptr<log4sp::base_logger> loggerAdapter;

    auto async = static_cast<bool>(params[4]);
    if (!async)
    {
        auto logger   = std::make_shared<spdlog::logger>(name, sinksList.begin(), sinksList.end());
        loggerAdapter = log4sp::sync_logger::create(logger, ctx);
    }
    else
    {
        auto policy   = log4sp::cell_to_policy(params[5]);
        auto distSink = std::make_shared<spdlog::sinks::dist_sink_mt>(sinksList);
        auto logger   = std::make_shared<spdlog::async_logger>(name, distSink, spdlog::thread_pool(), policy);
        loggerAdapter = log4sp::async_logger::create(logger, ctx);
    }

    if (loggerAdapter == nullptr)
    {
        return BAD_HANDLE;
    }

    return loggerAdapter->handle();
}

/**
 * public static native Logger Get(const char[] name);
 */
static cell_t Get(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);

    auto loggerAdapter = log4sp::logger_register::instance().get(name);
    return loggerAdapter == nullptr ? BAD_HANDLE : loggerAdapter->handle();
}

/**
 * public static native Logger CreateServerConsoleLogger(const char[] name,
 *                                                       bool async = false,
 *                                                       AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block);
 */
static cell_t CreateServerConsoleLogger(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_register::instance().get(name) != nullptr)
    {
        ctx->ReportError("Logger with name '%s' already exists.", name);
        return BAD_HANDLE;
    }

    std::shared_ptr<log4sp::base_logger> loggerAdapter;

    auto async = static_cast<bool>(params[2]);
    if (!async)
    {
        auto sink     = std::make_shared<spdlog::sinks::stdout_sink_st>();
        auto logger   = std::make_shared<spdlog::logger>(name, sink);
        loggerAdapter = log4sp::sync_logger::create(logger, ctx);
    }
    else
    {
        auto policy   = log4sp::cell_to_policy(params[3]);
        auto sink     = std::make_shared<spdlog::sinks::stdout_sink_st>();
        auto distSink = std::make_shared<spdlog::sinks::dist_sink_mt>(std::vector<spdlog::sink_ptr> {sink});
        auto logger   = std::make_shared<spdlog::async_logger>(name, distSink, spdlog::thread_pool(), policy);
        loggerAdapter = log4sp::async_logger::create(logger, ctx);
    }

    if (loggerAdapter == nullptr)
    {
        return BAD_HANDLE;
    }

    return loggerAdapter->handle();
}

/**
 * public static native Logger CreateBaseFileLogger(const char[] name,
 *                                                  const char[] file,
 *                                                  bool truncate = false,
 *                                                  bool async = false,
 *                                                  AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block
 * );
 */
static cell_t CreateBaseFileLogger(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_register::instance().get(name) != nullptr)
    {
        ctx->ReportError("Logger with name '%s' already exists.", name);
        return BAD_HANDLE;
    }

    char *file;
    ctx->LocalToString(params[2], &file);

    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    std::shared_ptr<log4sp::base_logger> loggerAdapter;

    auto truncate = static_cast<bool>(params[3]);
    auto async    = static_cast<bool>(params[4]);
    if (!async)
    {
        auto sink     = std::make_shared<spdlog::sinks::basic_file_sink_st>(path, truncate);
        auto logger   = std::make_shared<spdlog::logger>(name, sink);
        loggerAdapter = log4sp::sync_logger::create(logger, ctx);
    }
    else
    {
        auto policy   = log4sp::cell_to_policy(params[5]);
        auto sink     = std::make_shared<spdlog::sinks::basic_file_sink_st>(path, truncate);
        auto distSink = std::make_shared<spdlog::sinks::dist_sink_mt>(std::vector<spdlog::sink_ptr> {sink});
        auto logger   = std::make_shared<spdlog::async_logger>(name, distSink, spdlog::thread_pool(), policy);
        loggerAdapter = log4sp::async_logger::create(logger, ctx);
    }

    if (loggerAdapter == nullptr)
    {
        return BAD_HANDLE;
    }

    return loggerAdapter->handle();
}

/**
 * public static native Logger CreateRotatingFileLogger(const char[] name,
 *                                                      const char[] file,
 *                                                      int maxFileSize,
 *                                                      int maxFiles,
 *                                                      bool rotateOnOpen = false,
 *                                                      bool async = false,
 *                                                      AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block
 * );
 */
static cell_t CreateRotatingFileLogger(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_register::instance().get(name) != nullptr)
    {
        ctx->ReportError("Logger with name '%s' already exists.", name);
        return BAD_HANDLE;
    }

    char *file;
    ctx->LocalToString(params[2], &file);

    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto maxFileSize = static_cast<size_t>(params[3]);
    if (maxFileSize == 0)
    {
        ctx->ReportError("maxFileSize arg cannot be 0.");
        return BAD_HANDLE;
    }

    auto maxFiles = static_cast<size_t>(params[4]);
    if (maxFiles > 200000)
    {
        ctx->ReportError("maxFiles arg cannot exceed 200000.");
        return BAD_HANDLE;
    }

    std::shared_ptr<log4sp::base_logger> loggerAdapter;

    auto rotateOnOpen = static_cast<bool>(params[5]);
    auto async        = static_cast<bool>(params[6]);
    if (!async)
    {
        auto sink     = std::make_shared<spdlog::sinks::rotating_file_sink_st>(path, maxFileSize, maxFiles, rotateOnOpen);
        auto logger   = std::make_shared<spdlog::logger>(name, sink);
        loggerAdapter = log4sp::sync_logger::create(logger, ctx);
    }
    else
    {
        auto policy   = log4sp::cell_to_policy(params[7]);
        auto sink     = std::make_shared<spdlog::sinks::rotating_file_sink_st>(path, maxFileSize, maxFiles, rotateOnOpen);
        auto distSink = std::make_shared<spdlog::sinks::dist_sink_mt>(std::vector<spdlog::sink_ptr> {sink});
        auto logger   = std::make_shared<spdlog::async_logger>(name, distSink, spdlog::thread_pool(), policy);
        loggerAdapter = log4sp::async_logger::create(logger, ctx);
    }

    if (loggerAdapter == nullptr)
    {
        return BAD_HANDLE;
    }

    return loggerAdapter->handle();
}

/**
 * public static native Logger CreateDailyFileLogger(const char[] name,
 *                                                   const char[] file,
 *                                                   int hour = 0,
 *                                                   int minute = 0,
 *                                                   bool truncate = false,
 *                                                   int maxFiles = 0,
 *                                                   bool async = false,
 *                                                   AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block
 * );
 */
static cell_t CreateDailyFileLogger(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_register::instance().get(name) != nullptr)
    {
        ctx->ReportError("Logger with name '%s' already exists.", name);
        return BAD_HANDLE;
    }

    char *file;
    ctx->LocalToString(params[2], &file);

    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto hour   = static_cast<int>(params[3]);
    auto minute = static_cast<int>(params[4]);
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59)
    {
        ctx->ReportError("Invalid rotation time in ctor. (%d:%d)", hour, minute);
        return BAD_HANDLE;
    }

    std::shared_ptr<log4sp::base_logger> loggerAdapter;

    auto truncate = static_cast<bool>(params[5]);
    auto maxFiles = static_cast<uint16_t>(params[6]);
    auto async    = static_cast<bool>(params[7]);
    if (!async)
    {
        auto sink     = std::make_shared<spdlog::sinks::daily_file_sink_st>(path, hour, minute, truncate, maxFiles);
        auto logger   = std::make_shared<spdlog::logger>(name, sink);
        loggerAdapter = log4sp::sync_logger::create(logger, ctx);
    }
    else
    {
        auto policy   = log4sp::cell_to_policy(params[8]);
        auto sink     = std::make_shared<spdlog::sinks::daily_file_sink_st>(path, hour, minute, truncate, maxFiles);
        auto distSink = std::make_shared<spdlog::sinks::dist_sink_mt>(std::vector<spdlog::sink_ptr> {sink});
        auto logger   = std::make_shared<spdlog::async_logger>(name, distSink, spdlog::thread_pool(), policy);
        loggerAdapter = log4sp::async_logger::create(logger, ctx);
    }

    if (loggerAdapter == nullptr)
    {
        return BAD_HANDLE;
    }

    return loggerAdapter->handle();
}

/**
 * public native void GetName(char[] buffer, int maxlen);
 */
static cell_t GetName(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    ctx->StringToLocal(params[2], params[3], logger->raw()->name().c_str());
    return 0;
}

/**
 * public native LogLevel GetLevel();
 */
static cell_t GetLevel(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    return static_cast<cell_t>(logger->raw()->level());
}

/**
 * public native void SetLevel(LogLevel lvl);
 */
static cell_t SetLevel(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    logger->raw()->set_level(lvl);
    return 0;
}

/**
 * public native void SetPattern(const char[] pattern, PatternTimeType type = PatternTimeType_local);
 */
static cell_t SetPattern(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    char *pattern;
    ctx->LocalToString(params[2], &pattern);

    auto type = log4sp::cell_to_pattern_time_type(params[3]);

    logger->raw()->set_pattern(pattern, type);
    return 0;
}

/**
 * public native bool ShouldLog(LogLevel lvl);
 */
static cell_t ShouldLog(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    return logger->raw()->should_log(lvl);
}

/**
 * public native void Log(LogLevel lvl, const char[] msg);
 */
static cell_t Log(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    char *msg;
    ctx->LocalToString(params[3], &msg);

    logger->raw()->log(lvl, msg);
    return 0;
}

/**
 * public native void LogEx(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    std::string msg;
    try
    {
        msg = log4sp::format_cell_to_string(ctx, params, 3);
    }
    catch(const std::exception& e)
    {
        logger->error_handler(e.what());
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    logger->raw()->log(lvl, msg);
    return 0;
}

/**
 * public native void LogAmxTpl(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    char msg[2048];
    DetectExceptions eh(ctx);
    smutils->FormatString(msg, sizeof(msg), ctx, params, 3);
    if (eh.HasException())
    {
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    logger->raw()->log(lvl, msg);
    return 0;
}

/**
 * public native void LogSrc(LogLevel lvl, const char[] msg);
 */
static cell_t LogSrc(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);
    auto loc = log4sp::get_plugin_source_loc(ctx);

    char *msg;
    ctx->LocalToString(params[3], &msg);

    logger->raw()->log(loc, lvl, msg);
    return 0;
}

/**
 * public native void LogSrcEx(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogSrcEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    std::string msg;
    try
    {
        msg = log4sp::format_cell_to_string(ctx, params, 3);
    }
    catch(const std::exception& e)
    {
        logger->error_handler(e.what());
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);
    auto loc = log4sp::get_plugin_source_loc(ctx);

    logger->raw()->log(loc, lvl, msg);
    return 0;
}

/**
 * public native void LogSrcAmxTpl(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogSrcAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    char msg[2048];
    DetectExceptions eh(ctx);
    smutils->FormatString(msg, sizeof(msg), ctx, params, 3);
    if (eh.HasException())
    {
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);
    auto loc = log4sp::get_plugin_source_loc(ctx);

    logger->raw()->log(loc, lvl, msg);
    return 0;
}

/**
 * public native void LogLoc(const char[] file, int line, const char[] func, LogLevel lvl, const char[] msg);
 */
static cell_t LogLoc(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    char *file;
    ctx->LocalToString(params[2], &file);

    char *func;
    ctx->LocalToString(params[4], &func);

    char *msg;
    ctx->LocalToString(params[6], &msg);

    auto line = static_cast<int>(params[3]);
    auto lvl  = log4sp::cell_to_level(params[5]);
    auto loc  = spdlog::source_loc {file, line, func};

    logger->raw()->log(loc, lvl, msg);
    return 0;
}

/**
 * public native void LogLocEx(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogLocEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    std::string msg;
    try
    {
        msg = log4sp::format_cell_to_string(ctx, params, 6);
    }
    catch(const std::exception& e)
    {
        logger->error_handler(e.what());
        return 0;
    }

    char *file;
    ctx->LocalToString(params[2], &file);

    char *func;
    ctx->LocalToString(params[4], &func);

    auto line = static_cast<int>(params[3]);
    auto lvl  = log4sp::cell_to_level(params[5]);
    auto loc  = spdlog::source_loc {file, line, func};

    logger->raw()->log(loc, lvl, msg);
    return 0;
}

/**
 * public native void LogLocAmxTpl(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogLocAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    char msg[2048];
    DetectExceptions eh(ctx);
    smutils->FormatString(msg, sizeof(msg), ctx, params, 6);
    if (eh.HasException())
    {
        return 0;
    }

    char *file;
    ctx->LocalToString(params[2], &file);

    char *func;
    ctx->LocalToString(params[4], &func);

    auto line = static_cast<int>(params[3]);
    auto lvl  = log4sp::cell_to_level(params[5]);
    auto loc  = spdlog::source_loc {file, line, func};

    logger->raw()->log(loc, lvl, msg);
    return 0;
}

/**
 * public native void LogStackTrace(LogLevel lvl, const char[] msg);
 */
static cell_t LogStackTrace(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    char *msg;
    ctx->LocalToString(params[3], &msg);
    logger->raw()->log(lvl, "Stack trace requested: {}", msg);

    auto plugin = plsys->FindPluginByContext(ctx->GetContext())->GetFilename();
    logger->raw()->log(lvl, "Called from: {}", plugin);

    auto stackTrace = log4sp::get_stack_trace(ctx);
    for (auto iter  : stackTrace)
    {
        logger->raw()->log(lvl, iter.c_str());
    }
    return 0;
}

/**
 * public native void LogStackTraceEx(LogLevel lvl, const char[] msg, any ...);
 */
static cell_t LogStackTraceEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    std::string msg;
    try
    {
        msg = log4sp::format_cell_to_string(ctx, params, 3);
    }
    catch(const std::exception& e)
    {
        logger->error_handler(e.what());
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);
    logger->raw()->log(lvl, "Stack trace requested: {}", msg);

    auto plugin = plsys->FindPluginByContext(ctx->GetContext())->GetFilename();
    logger->raw()->log(lvl, "Called from: {}", plugin);

    auto stackTrace = log4sp::get_stack_trace(ctx);
    for (auto iter  : stackTrace)
    {
        logger->raw()->log(lvl, iter.c_str());
    }
    return 0;
}

/**
 * public native void LogStackTraceAmxTpl(LogLevel lvl, const char[] msg, any ...);
 */
static cell_t LogStackTraceAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    char msg[2048];
    DetectExceptions eh(ctx);
    smutils->FormatString(msg, sizeof(msg), ctx, params, 3);
    if (eh.HasException())
    {
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);
    logger->raw()->log(lvl, "Stack trace requested: {}", msg);

    auto plugin = plsys->FindPluginByContext(ctx->GetContext())->GetFilename();
    logger->raw()->log(lvl, "Called from: {}", plugin);

    auto stackTrace = log4sp::get_stack_trace(ctx);
    for (auto iter  : stackTrace)
    {
        logger->raw()->log(lvl, iter.c_str());
    }
    return 0;
}

/**
 * public native void ThrowError(LogLevel lvl, const char[] msg);
 */
static cell_t ThrowError(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    char *msg;
    ctx->LocalToString(params[3], &msg);
    logger->raw()->log(lvl, "Exception reported: {}", msg);

    auto plugin = plsys->FindPluginByContext(ctx->GetContext())->GetFilename();
    logger->raw()->log(lvl, "Blaming: {}", plugin);

    auto stackTrace = log4sp::get_stack_trace(ctx);
    for (auto iter  : stackTrace)
    {
        logger->raw()->log(lvl, iter.c_str());
    }

    ctx->ReportError(msg);
    return 0;
}

/**
 * public native void LogStackTraceEx(LogLevel lvl, const char[] msg, any ...);
 */
static cell_t ThrowErrorEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    std::string msg;
    try
    {
        msg = log4sp::format_cell_to_string(ctx, params, 3);
    }
    catch(const std::exception& e)
    {
        logger->error_handler(e.what());
        ctx->ReportError(e.what());
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);
    logger->raw()->log(lvl, "Exception reported: {}", msg);

    auto plugin = plsys->FindPluginByContext(ctx->GetContext())->GetFilename();
    logger->raw()->log(lvl, "Blaming: {}", plugin);

    auto stackTrace = log4sp::get_stack_trace(ctx);
    for (auto iter  : stackTrace)
    {
        logger->raw()->log(lvl, iter.c_str());
    }

    ctx->ReportError(msg.c_str());
    return 0;
}

/**
 * public native void ThrowErrorAmxTpl(LogLevel lvl, const char[] msg, any ...);
 */
static cell_t ThrowErrorAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    char msg[2048];
    DetectExceptions eh(ctx);
    smutils->FormatString(msg, sizeof(msg), ctx, params, 3);
    if (eh.HasException())
    {
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);
    logger->raw()->log(lvl, "Exception reported: {}", msg);

    auto plugin = plsys->FindPluginByContext(ctx->GetContext())->GetFilename();
    logger->raw()->log(lvl, "Blaming: {}", plugin);

    auto stackTrace = log4sp::get_stack_trace(ctx);
    for (auto iter  : stackTrace)
    {
        logger->raw()->log(lvl, iter.c_str());
    }

    ctx->ReportError(msg);
    return 0;
}

/**
 * public native void Trace(const char[] msg);
 */
static cell_t Trace(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->raw()->trace(msg);
    return 0;
}

/**
 * public native void TraceEx(const char[] fmt, any ...);
 */
static cell_t TraceEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    std::string msg;
    try
    {
        msg = log4sp::format_cell_to_string(ctx, params, 2);
    }
    catch(const std::exception& e)
    {
        logger->error_handler(e.what());
        return 0;
    }

    logger->raw()->trace(msg);
    return 0;
}

/**
 * public native void TraceAmxTpl(const char[] fmt, any ...);
 */
static cell_t TraceAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    char msg[2048];
    DetectExceptions eh(ctx);
    smutils->FormatString(msg, sizeof(msg), ctx, params, 2);
    if (eh.HasException())
    {
        return 0;
    }

    logger->raw()->trace(msg);
    return 0;
}

/**
 * public native void Debug(const char[] msg);
 */
static cell_t Debug(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->raw()->debug(msg);
    return 0;
}

/**
 * public native void DebugEx(const char[] fmt, any ...);
 */
static cell_t DebugEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    std::string msg;
    try
    {
        msg = log4sp::format_cell_to_string(ctx, params, 2);
    }
    catch(const std::exception& e)
    {
        logger->error_handler(e.what());
        return 0;
    }

    logger->raw()->debug(msg);
    return 0;
}

/**
 * public native void DebugAmxTpl(const char[] fmt, any ...);
 */
static cell_t DebugAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    char msg[2048];
    DetectExceptions eh(ctx);
    smutils->FormatString(msg, sizeof(msg), ctx, params, 2);
    if (eh.HasException())
    {
        return 0;
    }

    logger->raw()->debug(msg);
    return 0;
}

/**
 * public native void Info(const char[] msg);
 */
static cell_t Info(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->raw()->info(msg);
    return 0;
}

/**
 * public native void InfoEx(const char[] fmt, any ...);
 */
static cell_t InfoEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    std::string msg;
    try
    {
        msg = log4sp::format_cell_to_string(ctx, params, 2);
    }
    catch(const std::exception& e)
    {
        logger->error_handler(e.what());
        return 0;
    }

    logger->raw()->info(msg);
    return 0;
}

/**
 * public native void InfoAmxTpl(const char[] fmt, any ...);
 */
static cell_t InfoAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    char msg[2048];
    DetectExceptions eh(ctx);
    smutils->FormatString(msg, sizeof(msg), ctx, params, 2);
    if (eh.HasException())
    {
        return 0;
    }

    logger->raw()->info(msg);
    return 0;
}

/**
 * public native void Warn(const char[] msg);
 */
static cell_t Warn(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->raw()->warn(msg);
    return 0;
}

/**
 * public native void WarnEx(const char[] fmt, any ...);
 */
static cell_t WarnEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    std::string msg;
    try
    {
        msg = log4sp::format_cell_to_string(ctx, params, 2);
    }
    catch(const std::exception& e)
    {
        logger->error_handler(e.what());
        return 0;
    }

    logger->raw()->warn(msg);
    return 0;
}

/**
 * public native void WarnAmxTpl(const char[] fmt, any ...);
 */
static cell_t WarnAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    char msg[2048];
    DetectExceptions eh(ctx);
    smutils->FormatString(msg, sizeof(msg), ctx, params, 2);
    if (eh.HasException())
    {
        return 0;
    }

    logger->raw()->warn(msg);
    return 0;
}

/**
 * public native void Error(const char[] msg);
 */
static cell_t Error(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->raw()->error(msg);
    return 0;
}

/**
 * public native void ErrorEx(const char[] fmt, any ...);
 */
static cell_t ErrorEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    std::string msg;
    try
    {
        msg = log4sp::format_cell_to_string(ctx, params, 2);
    }
    catch(const std::exception& e)
    {
        logger->error_handler(e.what());
        return 0;
    }

    logger->raw()->error(msg);
    return 0;
}

/**
 * public native void ErrorAmxTpl(const char[] fmt, any ...);
 */
static cell_t ErrorAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    char msg[2048];
    DetectExceptions eh(ctx);
    smutils->FormatString(msg, sizeof(msg), ctx, params, 2);
    if (eh.HasException())
    {
        return 0;
    }

    logger->raw()->error(msg);
    return 0;
}

/**
 * public native void Fatal(const char[] msg);
 */
static cell_t Fatal(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->raw()->critical(msg);
    return 0;
}

/**
 * public native void FatalEx(const char[] fmt, any ...);
 */
static cell_t FatalEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    std::string msg;
    try
    {
        msg = log4sp::format_cell_to_string(ctx, params, 2);
    }
    catch(const std::exception& e)
    {
        logger->error_handler(e.what());
        return 0;
    }

    logger->raw()->critical(msg);
    return 0;
}

/**
 * public native void FatalAmxTpl(const char[] fmt, any ...);
 */
static cell_t FatalAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    char msg[2048];
    DetectExceptions eh(ctx);
    smutils->FormatString(msg, sizeof(msg), ctx, params, 2);
    if (eh.HasException())
    {
        return 0;
    }

    logger->raw()->critical(msg);
    return 0;
}

/**
 * public native void Flush();
 */
static cell_t Flush(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    logger->raw()->flush();
    return 0;
}

/**
 * public native LogLevel GetFlushLevel();
 */
static cell_t GetFlushLevel(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    return static_cast<cell_t>(logger->raw()->flush_level());
}

/**
 * public native void FlushOn(LogLevel lvl);
 */
static cell_t FlushOn(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    logger->raw()->flush_on(lvl);
    return 0;
}

/**
 * public native bool ShouldBacktrace();
 */
static cell_t ShouldBacktrace(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    return static_cast<cell_t>(logger->raw()->should_backtrace());
}

/**
 * public native void EnableBacktrace(int num);
 */
static cell_t EnableBacktrace(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    auto num = static_cast<size_t>(params[2]);

    logger->raw()->enable_backtrace(num);
    return 0;
}

/**
 * public native void DisableBacktrace();
 */
static cell_t DisableBacktrace(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    logger->raw()->disable_backtrace();
    return 0;
}

/**
 * public native void DumpBacktrace();
 */
static cell_t DumpBacktrace(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    logger->raw()->dump_backtrace();
    return 0;
}

/**
 * public native void AddSink(Sink sink);
 */
static cell_t AddSink(IPluginContext *ctx, const cell_t *params)
{
    auto loggerHandle = static_cast<Handle_t>(params[1]);
    auto loggerAdapterRaw = log4sp::base_logger::read(loggerHandle, ctx);
    if (loggerAdapterRaw == nullptr)
    {
        return 0;
    }

    auto sinkHandle = static_cast<Handle_t>(params[2]);
    auto sinkAdapterRaw = log4sp::base_sink::read(sinkHandle, ctx);
    if (sinkAdapterRaw == nullptr)
    {
        return 0;
    }

    loggerAdapterRaw->add_sink(sinkAdapterRaw->raw());
    return 0;
}

/**
 * public native void DropSink(Sink sink);
 */
static cell_t DropSink(IPluginContext *ctx, const cell_t *params)
{
    auto loggerHandle = static_cast<Handle_t>(params[1]);
    auto loggerAdapterRaw = log4sp::base_logger::read(loggerHandle, ctx);
    if (loggerAdapterRaw == nullptr)
    {
        return 0;
    }

    auto sinkHandle = static_cast<Handle_t>(params[2]);
    auto sinkAdapterRaw = log4sp::base_sink::read(sinkHandle, ctx);
    if (sinkAdapterRaw == nullptr)
    {
        return 0;
    }

    loggerAdapterRaw->remove_sink(sinkAdapterRaw->raw());
    return 0;
}

/**
 * public native void SetErrorHandler(LoggerErrorHandler handler);
 *
 * function void (const char[] msg);
 */
static cell_t SetErrorHandler(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::base_logger::read(handle, ctx);
    if (logger == nullptr)
    {
        return 0;
    }

    auto funcId   = static_cast<funcid_t>(params[2]);
    auto function = ctx->GetFunctionById(funcId);
    if (function == nullptr)
    {
        ctx->ReportError("Invalid error handler function. (funcId=%d)", static_cast<int>(funcId));
        return 0;
    }

    auto forward = forwards->CreateForwardEx(nullptr, ET_Ignore, 1, nullptr, Param_String);
    if (forward == nullptr)
    {
        ctx->ReportError("SM error! Create error handler forward failure.");
        return 0;
    }

    if (!forward->AddFunction(function))
    {
        forwards->ReleaseForward(forward);
        ctx->ReportError("SM error! Adding error handler function failed.");
        return 0;
    }

    logger->set_error_forward(forward);
    return 0;
}

const sp_nativeinfo_t LoggerNatives[] =
{
    {"Logger.Logger",                           Logger},
    {"Logger.Get",                              Get},
    {"Logger.CreateServerConsoleLogger",        CreateServerConsoleLogger},
    {"Logger.CreateBaseFileLogger",             CreateBaseFileLogger},
    {"Logger.CreateRotatingFileLogger",         CreateRotatingFileLogger},
    {"Logger.CreateDailyFileLogger",            CreateDailyFileLogger},
    {"Logger.GetName",                          GetName},
    {"Logger.GetLevel",                         GetLevel},
    {"Logger.SetLevel",                         SetLevel},
    {"Logger.SetPattern",                       SetPattern},
    {"Logger.ShouldLog",                        ShouldLog},

    {"Logger.Log",                              Log},
    {"Logger.LogEx",                            LogEx},
    {"Logger.LogAmxTpl",                        LogAmxTpl},
    {"Logger.LogSrc",                           LogSrc},
    {"Logger.LogSrcEx",                         LogSrcEx},
    {"Logger.LogSrcAmxTpl",                     LogSrcAmxTpl},
    {"Logger.LogLoc",                           LogLoc},
    {"Logger.LogLocEx",                         LogLocEx},
    {"Logger.LogLocAmxTpl",                     LogLocAmxTpl},
    {"Logger.LogStackTrace",                    LogStackTrace},
    {"Logger.LogStackTraceEx",                  LogStackTraceEx},
    {"Logger.LogStackTraceAmxTpl",              LogStackTraceAmxTpl},
    {"Logger.ThrowError",                       ThrowError},
    {"Logger.ThrowErrorEx",                     ThrowErrorEx},
    {"Logger.ThrowErrorAmxTpl",                 ThrowErrorAmxTpl},

    {"Logger.Trace",                            Trace},
    {"Logger.TraceEx",                          TraceEx},
    {"Logger.TraceAmxTpl",                      TraceAmxTpl},
    {"Logger.Debug",                            Debug},
    {"Logger.DebugEx",                          DebugEx},
    {"Logger.DebugAmxTpl",                      DebugAmxTpl},
    {"Logger.Info",                             Info},
    {"Logger.InfoEx",                           InfoEx},
    {"Logger.InfoAmxTpl",                       InfoAmxTpl},
    {"Logger.Warn",                             Warn},
    {"Logger.WarnEx",                           WarnEx},
    {"Logger.WarnAmxTpl",                       WarnAmxTpl},
    {"Logger.Error",                            Error},
    {"Logger.ErrorEx",                          ErrorEx},
    {"Logger.ErrorAmxTpl",                      ErrorAmxTpl},
    {"Logger.Fatal",                            Fatal},
    {"Logger.FatalEx",                          FatalEx},
    {"Logger.FatalAmxTpl",                      FatalAmxTpl},

    {"Logger.Flush",                            Flush},
    {"Logger.GetFlushLevel",                    GetFlushLevel},
    {"Logger.FlushOn",                          FlushOn},
    {"Logger.ShouldBacktrace",                  ShouldBacktrace},
    {"Logger.EnableBacktrace",                  EnableBacktrace},
    {"Logger.DisableBacktrace",                 DisableBacktrace},
    {"Logger.DumpBacktrace",                    DumpBacktrace},
    {"Logger.AddSink",                          AddSink},
    {"Logger.DropSink",                         DropSink},
    {"Logger.SetErrorHandler",                  SetErrorHandler},

    {nullptr,                                   nullptr}
};


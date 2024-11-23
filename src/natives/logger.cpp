#include "spdlog/async.h"
#include "spdlog/sinks/dist_sink.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"

#include <log4sp/utils.h>
#include <log4sp/logger_handle_manager.h>
#include <log4sp/sink_handle_manager.h>

/**
 * Thread safe logger (except for set_error_handler())
 * Has name, log level, vector of std::shared sink pointers and formatter
 * Upon each log write the logger:
 * 1. Checks if its log level is enough to log the message and if yes:
 * 2. Call the underlying sinks to do the job.
 * 3. Each sink use its own private copy of a formatter to format the message
 * and send to its destination.
 *
 * The use of private formatter per sink provides the opportunity to cache some
 * formatted data, and support for different format per sink.
 */

/**
 * public native Logger(const char[] name, Sink[] sinks, int numSinks, bool async = false, AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block);
 */
static cell_t Logger(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (!strcmp(name, SMEXT_CONF_LOGTAG))
    {
        ctx->ReportError("'" SMEXT_CONF_LOGTAG "' is a reserved dedicated logger name.");
        return BAD_HANDLE;
    }

    if (log4sp::logger_handle_manager::instance().get_data(name) != nullptr)
    {
        ctx->ReportError("Logger with name '%s' already exists.", name);
        return BAD_HANDLE;
    }

    cell_t *sinks;
    ctx->LocalToPhysAddr(params[2], &sinks);

    auto numSinks = static_cast<unsigned int>(params[3]);
    auto async = static_cast<bool>(params[4]);

    std::vector<spdlog::sink_ptr> sinksList;
    for (unsigned int i = 0; i < numSinks; ++i)
    {
        auto sinkHandle = static_cast<Handle_t>(sinks[i]);
        auto sink = log4sp::sink_handle_manager::instance().read_handle(ctx, sinkHandle);
        if (sink == nullptr)
        {
            return BAD_HANDLE;
        }

        auto data = log4sp::sink_handle_manager::instance().get_data(sink);
        if (data == nullptr)
        {
            ctx->ReportError("Fatal internal error, sink data not found. (hdl=%X)", static_cast<int>(sinkHandle));
            return BAD_HANDLE;
        }

        if (data->is_multi_threaded() != async)
        {
            ctx->ReportError("You cannot create a %s logger with a %s sink.", async ? "asynchronous" : "synchronous", async ? "synchronous" : "asynchronous");
            return BAD_HANDLE;
        }

        sinksList.push_back(data->sink_ptr());
    }

    if (!async)
    {
        auto data = log4sp::logger_handle_manager::instance().create_logger_st(ctx, name, sinksList);
        return data->handle();
    }

    auto policy = static_cast<spdlog::async_overflow_policy>(params[5]);
    if (policy < static_cast<spdlog::async_overflow_policy>(0))
    {
        policy = static_cast<spdlog::async_overflow_policy>(0);
    }
    else if (policy > spdlog::async_overflow_policy::discard_new)
    {
        policy = spdlog::async_overflow_policy::discard_new;
    }

    auto data = log4sp::logger_handle_manager::instance().create_logger_mt(ctx, name, sinksList, policy);
    return data->handle();
}

/**
 * public static native Logger Get(const char[] name);
 */
static cell_t Get(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (!strcmp(name, SMEXT_CONF_LOGTAG))
    {
        ctx->ReportError("The logger named '" SMEXT_CONF_LOGTAG "' is reserved dedicated logger.");
        return BAD_HANDLE;
    }

    auto data = log4sp::logger_handle_manager::instance().get_data(name);
    if (data == nullptr)
    {
        ctx->ReportError("Logger named '%s' does not exist.", name);
        return BAD_HANDLE;
    }

    return data->handle();
}

/**
 * public static native Logger CreateServerConsoleLogger(const char[] name, bool async = false, AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block);
 */
static cell_t CreateServerConsoleLogger(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (!strcmp(name, SMEXT_CONF_LOGTAG))
    {
        ctx->ReportError("'" SMEXT_CONF_LOGTAG "' is a reserved dedicated logger name.");
        return BAD_HANDLE;
    }

    if (log4sp::logger_handle_manager::instance().get_data(name) != nullptr)
    {
        ctx->ReportError("Logger with name '%s' already exists.", name);
        return BAD_HANDLE;
    }

    auto async = static_cast<bool>(params[2]);
    if (!async)
    {
        auto data = log4sp::logger_handle_manager::instance().create_server_console_logger_st(ctx, name);
        return data->handle();
    }

    auto policy = static_cast<spdlog::async_overflow_policy>(params[3]);
    if (policy < static_cast<spdlog::async_overflow_policy>(0))
    {
        policy = static_cast<spdlog::async_overflow_policy>(0);
    }
    else if (policy > spdlog::async_overflow_policy::discard_new)
    {
        policy = spdlog::async_overflow_policy::discard_new;
    }

    auto data = log4sp::logger_handle_manager::instance().create_server_console_logger_mt(ctx, name, policy);
    return data->handle();
}

/**
 * public static native Logger CreateBaseFileLogger(
 *     const char[] name,
 *     const char[] file,
 *     bool truncate = false,
 *     bool async = false,
 *     AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block
 * );
 */
static cell_t CreateBaseFileLogger(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (!strcmp(name, SMEXT_CONF_LOGTAG))
    {
        ctx->ReportError("'" SMEXT_CONF_LOGTAG "' is a reserved dedicated logger name.");
        return BAD_HANDLE;
    }

    if (log4sp::logger_handle_manager::instance().get_data(name) != nullptr)
    {
        ctx->ReportError("Logger with name '%s' already exists.", name);
        return BAD_HANDLE;
    }

    char *file;
    ctx->LocalToString(params[2], &file);
    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto truncate = static_cast<bool>(params[3]);
    auto async = static_cast<bool>(params[4]);
    if (!async)
    {
        auto data = log4sp::logger_handle_manager::instance().create_base_file_logger_st(ctx, name, path, truncate);
        return data->handle();
    }

    auto policy = static_cast<spdlog::async_overflow_policy>(params[5]);
    if (policy < static_cast<spdlog::async_overflow_policy>(0))
    {
        policy = static_cast<spdlog::async_overflow_policy>(0);
    }
    else if (policy > spdlog::async_overflow_policy::discard_new)
    {
        policy = spdlog::async_overflow_policy::discard_new;
    }

    auto data = log4sp::logger_handle_manager::instance().create_base_file_logger_mt(ctx, name, path, truncate, policy);
    return data->handle();
}

/**
 * public static native Logger CreateRotatingFileLogger(
 *      const char[] name,
 *      const char[] file,
 *      int maxFileSize,
 *      int maxFiles,
 *      bool rotateOnOpen = false,
 *      bool async = false,
 *      AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block
 * );
 */
static cell_t CreateRotatingFileLogger(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (!strcmp(name, SMEXT_CONF_LOGTAG))
    {
        ctx->ReportError("'" SMEXT_CONF_LOGTAG "' is a reserved dedicated logger name.");
        return BAD_HANDLE;
    }

    if (log4sp::logger_handle_manager::instance().get_data(name) != nullptr)
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

    auto rotateOnOpen = static_cast<bool>(params[5]);
    auto async = static_cast<bool>(params[6]);
    if (!async)
    {
        auto data = log4sp::logger_handle_manager::instance().create_rotating_file_logger_st(ctx, name, path, maxFileSize, maxFiles, rotateOnOpen);
        return data->handle();
    }

    auto policy = static_cast<spdlog::async_overflow_policy>(params[7]);
    if (policy < static_cast<spdlog::async_overflow_policy>(0))
    {
        policy = static_cast<spdlog::async_overflow_policy>(0);
    }
    else if (policy > spdlog::async_overflow_policy::discard_new)
    {
        policy = spdlog::async_overflow_policy::discard_new;
    }

    auto data = log4sp::logger_handle_manager::instance().create_rotating_file_logger_mt(ctx, name, path, maxFileSize, maxFiles, rotateOnOpen, policy);
    return data->handle();
}

/**
 * public static native Logger CreateDailyFileLogger(
 *      const char[] name,
 *      const char[] file,
 *      int hour = 0,
 *      int minute = 0,
 *      bool truncate = false,
 *      int maxFiles = 0,
 *      bool async = false,
 *      AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block
 * );
 */
static cell_t CreateDailyFileLogger(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (!strcmp(name, SMEXT_CONF_LOGTAG))
    {
        ctx->ReportError("'" SMEXT_CONF_LOGTAG "' is a reserved dedicated logger name.");
        return BAD_HANDLE;
    }

    if (log4sp::logger_handle_manager::instance().get_data(name) != nullptr)
    {
        ctx->ReportError("Logger with name '%s' already exists.", name);
        return BAD_HANDLE;
    }

    char *file;
    ctx->LocalToString(params[2], &file);
    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto hour = static_cast<int>(params[3]);
    auto minute = static_cast<int>(params[4]);
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59)
    {
        ctx->ReportError("Invalid rotation time in ctor. (%d:%d)", hour, minute);
        return BAD_HANDLE;
    }

    auto truncate = static_cast<bool>(params[5]);
    auto maxFiles = static_cast<uint16_t>(params[6]);
    auto async = static_cast<bool>(params[7]);
    if (!async)
    {
        auto data = log4sp::logger_handle_manager::instance().create_daily_file_logger_st(ctx, name, path, hour, minute, truncate, maxFiles);
        return data->handle();
    }

    auto policy = static_cast<spdlog::async_overflow_policy>(params[8]);
    if (policy < static_cast<spdlog::async_overflow_policy>(0))
    {
        policy = static_cast<spdlog::async_overflow_policy>(0);
    }
    else if (policy > spdlog::async_overflow_policy::discard_new)
    {
        policy = spdlog::async_overflow_policy::discard_new;
    }

    auto data = log4sp::logger_handle_manager::instance().create_daily_file_logger_mt(ctx, name, path, hour, minute, truncate, maxFiles, policy);
    return data->handle();
}

/**
 * public native void GetName(char[] buffer, int maxlen);
 */
static cell_t GetName(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    ctx->StringToLocal(params[2], params[3], logger->name().c_str());
    return 0;
}

/**
 * public native LogLevel GetLevel();
 */
static cell_t GetLevel(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    return logger->level();
}

/**
 * public native void SetLevel(LogLevel lvl);
 */
static cell_t SetLevel(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    auto lvl = static_cast<spdlog::level::level_enum>(params[2]);
    if (lvl < static_cast<spdlog::level::level_enum>(0))
    {
        lvl = static_cast<spdlog::level::level_enum>(0);
    }
    else if (lvl >= spdlog::level::n_levels)
    {
        lvl = static_cast<spdlog::level::level_enum>(spdlog::level::n_levels - 1);
    }

    logger->set_level(lvl);
    return 0;
}

/**
 * set formatting for the sinks in this logger.
 * each sink will get a separate instance of the formatter object.
 *
 * set formatting for the sinks in this logger.
 * equivalent to
 *     set_formatter(make_unique<pattern_formatter>(pattern, time_type))
 * Note: each sink will get a new instance of a formatter object, replacing the old one.
 *
 * pattern flags (https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags)
 */
/**
 * public native void SetPattern(const char[] pattern, PatternTimeType type = PatternTimeType_local);
 */
static cell_t SetPattern(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    char *pattern;
    ctx->LocalToString(params[2], &pattern);

    auto type = static_cast<spdlog::pattern_time_type>(params[3]);
    if (type < static_cast<spdlog::pattern_time_type>(0))
    {
        type = static_cast<spdlog::pattern_time_type>(0);
    }
    else if (type > spdlog::pattern_time_type::utc)
    {
        type = spdlog::pattern_time_type::utc;
    }

    logger->set_pattern(pattern, type);
    return 0;
}

/**
 * public native bool ShouldLog(LogLevel lvl);
 */
static cell_t ShouldLog(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    auto lvl = static_cast<spdlog::level::level_enum>(params[2]);
    if (lvl < static_cast<spdlog::level::level_enum>(0))
    {
        lvl = static_cast<spdlog::level::level_enum>(0);
    }
    else if (lvl >= spdlog::level::n_levels)
    {
        lvl = static_cast<spdlog::level::level_enum>(spdlog::level::n_levels - 1);
    }

    return logger->should_log(lvl);
}

/**
 * public native void Log(LogLevel lvl, const char[] msg);
 */
static cell_t Log(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    auto lvl = static_cast<spdlog::level::level_enum>(params[2]);
    if (lvl < static_cast<spdlog::level::level_enum>(0))
    {
        lvl = static_cast<spdlog::level::level_enum>(0);
    }
    else if (lvl >= spdlog::level::n_levels)
    {
        lvl = static_cast<spdlog::level::level_enum>(spdlog::level::n_levels - 1);
    }

    char *msg;
    ctx->LocalToString(params[3], &msg);

    logger->log(lvl, msg);
    return 0;
}

/**
 * public native void LogAmxTpl(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    auto lvl = static_cast<spdlog::level::level_enum>(params[2]);
    if (lvl < static_cast<spdlog::level::level_enum>(0))
    {
        lvl = static_cast<spdlog::level::level_enum>(0);
    }
    else if (lvl >= spdlog::level::n_levels)
    {
        lvl = static_cast<spdlog::level::level_enum>(spdlog::level::n_levels - 1);
    }

    std::string msg;
    try
    {
        msg = log4sp::FormatToAmxTplString(ctx, params, 3);
    }
    catch(const std::exception& e)
    {
        spdlog::source_loc loc = log4sp::GetScriptedLoc(ctx);
        logger->log(loc, spdlog::level::err, e.what());
        spdlog::log(loc, spdlog::level::err, e.what());
        return 0;
    }

    logger->log(lvl, msg);
    return 0;
}

/**
 * public native void LogSrc(LogLevel lvl, const char[] msg);
 */
static cell_t LogSrc(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    auto lvl = static_cast<spdlog::level::level_enum>(params[2]);
    if (lvl < static_cast<spdlog::level::level_enum>(0))
    {
        lvl = static_cast<spdlog::level::level_enum>(0);
    }
    else if (lvl >= spdlog::level::n_levels)
    {
        lvl = static_cast<spdlog::level::level_enum>(spdlog::level::n_levels - 1);
    }

    char *msg;
    ctx->LocalToString(params[3], &msg);

    auto loc = log4sp::GetScriptedLoc(ctx);
    logger->log(loc, lvl, msg);
    return 0;
}

/**
 * public native void LogSrcAmxTpl(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogSrcAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    auto lvl = static_cast<spdlog::level::level_enum>(params[2]);
    if (lvl < static_cast<spdlog::level::level_enum>(0))
    {
        lvl = static_cast<spdlog::level::level_enum>(0);
    }
    else if (lvl >= spdlog::level::n_levels)
    {
        lvl = static_cast<spdlog::level::level_enum>(spdlog::level::n_levels - 1);
    }

    std::string msg;
    auto loc = log4sp::GetScriptedLoc(ctx);

    try
    {
        msg = log4sp::FormatToAmxTplString(ctx, params, 3);
    }
    catch(const std::exception& e)
    {
        logger->log(loc, spdlog::level::err, e.what());
        spdlog::log(loc, spdlog::level::err, e.what());
        return 0;
    }

    logger->log(loc, lvl, msg);
    return 0;
}

/**
 * public native void LogLoc(const char[] file, int line, const char[] func, LogLevel lvl, const char[] msg);
 */
static cell_t LogLoc(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    char *file;
    ctx->LocalToString(params[2], &file);

    auto line = static_cast<int>(params[3]);

    char *func;
    ctx->LocalToString(params[4], &func);

    auto lvl = static_cast<spdlog::level::level_enum>(params[5]);
    if (lvl < static_cast<spdlog::level::level_enum>(0))
    {
        lvl = static_cast<spdlog::level::level_enum>(0);
    }
    else if (lvl >= spdlog::level::n_levels)
    {
        lvl = static_cast<spdlog::level::level_enum>(spdlog::level::n_levels - 1);
    }

    char *msg;
    ctx->LocalToString(params[6], &msg);

    auto loc = spdlog::source_loc{file, line, func};
    logger->log(loc, lvl, msg);
    return 0;
}

/**
 * public native void LogLocAmxTpl(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogLocAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    char *file;
    ctx->LocalToString(params[2], &file);

    auto line = static_cast<int>(params[3]);

    char *func;
    ctx->LocalToString(params[4], &func);

    auto lvl = static_cast<spdlog::level::level_enum>(params[5]);
    if (lvl < static_cast<spdlog::level::level_enum>(0))
    {
        lvl = static_cast<spdlog::level::level_enum>(0);
    }
    else if (lvl >= spdlog::level::n_levels)
    {
        lvl = static_cast<spdlog::level::level_enum>(spdlog::level::n_levels - 1);
    }

    std::string msg;
    try
    {
        msg = log4sp::FormatToAmxTplString(ctx, params, 6);
    }
    catch(const std::exception& e)
    {
        spdlog::source_loc loc = log4sp::GetScriptedLoc(ctx);
        logger->log(loc, spdlog::level::err, e.what());
        spdlog::log(loc, spdlog::level::err, e.what());
        return 0;
    }

    auto loc = spdlog::source_loc{file, line, func};
    logger->log(loc, lvl, msg);
    return 0;
}

/**
 * public native void LogStackTrace(LogLevel lvl, const char[] msg);
 */
static cell_t LogStackTrace(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    auto lvl = static_cast<spdlog::level::level_enum>(params[2]);
    if (lvl < static_cast<spdlog::level::level_enum>(0))
    {
        lvl = static_cast<spdlog::level::level_enum>(0);
    }
    else if (lvl >= spdlog::level::n_levels)
    {
        lvl = static_cast<spdlog::level::level_enum>(spdlog::level::n_levels - 1);
    }

    char *msg;
    ctx->LocalToString(params[3], &msg);
    logger->log(lvl, "Stack trace requested: {}", msg);

    auto plugin = plsys->FindPluginByContext(ctx->GetContext());
    logger->log(lvl, "Called from: {}", plugin->GetFilename());

    auto arr = log4sp::GetStackTrace(ctx);
    for (size_t i = 0; i < arr.size(); ++i)
    {
        logger->log(lvl, arr[i].c_str());
    }
    return 0;
}

/**
 * public native void LogStackTraceAmxTpl(LogLevel lvl, const char[] msg, any ...);
 */
static cell_t LogStackTraceAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    auto lvl = static_cast<spdlog::level::level_enum>(params[2]);
    if (lvl < static_cast<spdlog::level::level_enum>(0))
    {
        lvl = static_cast<spdlog::level::level_enum>(0);
    }
    else if (lvl >= spdlog::level::n_levels)
    {
        lvl = static_cast<spdlog::level::level_enum>(spdlog::level::n_levels - 1);
    }

    std::string msg;
    try
    {
        msg = log4sp::FormatToAmxTplString(ctx, params, 3);
    }
    catch(const std::exception& e)
    {
        spdlog::source_loc loc = log4sp::GetScriptedLoc(ctx);
        logger->log(loc, spdlog::level::err, e.what());
        spdlog::log(loc, spdlog::level::err, e.what());
        return 0;
    }

    logger->log(lvl, "Stack trace requested: {}", msg);

    auto plugin = plsys->FindPluginByContext(ctx->GetContext());
    logger->log(lvl, "Called from: {}", plugin->GetFilename());

    auto arr = log4sp::GetStackTrace(ctx);
    for (size_t i = 0; i < arr.size(); ++i)
    {
        logger->log(lvl, arr[i].c_str());
    }
    return 0;
}

/**
 * public native void ThrowError(LogLevel lvl, const char[] msg);
 */
static cell_t ThrowError(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    auto lvl = static_cast<spdlog::level::level_enum>(params[2]);
    if (lvl < static_cast<spdlog::level::level_enum>(0))
    {
        lvl = static_cast<spdlog::level::level_enum>(0);
    }
    else if (lvl >= spdlog::level::n_levels)
    {
        lvl = static_cast<spdlog::level::level_enum>(spdlog::level::n_levels - 1);
    }

    char *msg;
    ctx->LocalToString(params[3], &msg);
    logger->log(lvl, "Exception reported: {}", msg);

    auto plugin = plsys->FindPluginByContext(ctx->GetContext());
    logger->log(lvl, "Blaming: {}", plugin->GetFilename());

    auto arr = log4sp::GetStackTrace(ctx);
    for (size_t i = 0; i < arr.size(); ++i)
    {
        logger->log(lvl, arr[i].c_str());
    }

    ctx->ReportError(msg);
    return 0;
}

/**
 * public native void LogStackTraceAmxTpl(LogLevel lvl, const char[] msg, any ...);
 */
static cell_t ThrowErrorAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    auto lvl = static_cast<spdlog::level::level_enum>(params[2]);
    if (lvl < static_cast<spdlog::level::level_enum>(0))
    {
        lvl = static_cast<spdlog::level::level_enum>(0);
    }
    else if (lvl >= spdlog::level::n_levels)
    {
        lvl = static_cast<spdlog::level::level_enum>(spdlog::level::n_levels - 1);
    }

    std::string msg;
    try
    {
        msg = log4sp::FormatToAmxTplString(ctx, params, 3);
    }
    catch(const std::exception& e)
    {
        spdlog::source_loc loc = log4sp::GetScriptedLoc(ctx);
        logger->log(loc, spdlog::level::err, e.what());
        spdlog::log(loc, spdlog::level::err, e.what());
        ctx->ReportError(e.what());
        return 0;
    }

    logger->log(lvl, "Exception reported: {}", msg);

    auto plugin = plsys->FindPluginByContext(ctx->GetContext());
    logger->log(lvl, "Blaming: {}", plugin->GetFilename());

    auto arr = log4sp::GetStackTrace(ctx);
    for (size_t i = 0; i < arr.size(); ++i)
    {
        logger->log(lvl, arr[i].c_str());
    }

    ctx->ReportError(msg.c_str());
    return 0;
}

/**
 * public native void Trace(const char[] msg);
 */
static cell_t Trace(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->trace(msg);
    return 0;
}

/**
 * public native void TraceAmxTpl(const char[] fmt, any ...);
 */
static cell_t TraceAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    std::string msg;
    try
    {
        msg = log4sp::FormatToAmxTplString(ctx, params, 2);
    }
    catch(const std::exception& e)
    {
        spdlog::source_loc loc = log4sp::GetScriptedLoc(ctx);
        logger->log(loc, spdlog::level::err, e.what());
        spdlog::log(loc, spdlog::level::err, e.what());
        return 0;
    }

    logger->trace(msg);
    return 0;
}

/**
 * public native void Debug(const char[] msg);
 */
static cell_t Debug(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->debug(msg);
    return 0;
}

/**
 * public native void DebugAmxTpl(const char[] fmt, any ...);
 */
static cell_t DebugAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    std::string msg;
    try
    {
        msg = log4sp::FormatToAmxTplString(ctx, params, 2);
    }
    catch(const std::exception& e)
    {
        spdlog::source_loc loc = log4sp::GetScriptedLoc(ctx);
        logger->log(loc, spdlog::level::err, e.what());
        spdlog::log(loc, spdlog::level::err, e.what());
        return 0;
    }

    logger->debug(msg);
    return 0;
}

/**
 * public native void Info(const char[] msg);
 */
static cell_t Info(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->info(msg);
    return 0;
}

/**
 * public native void InfoAmxTpl(const char[] fmt, any ...);
 */
static cell_t InfoAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    std::string msg;
    try
    {
        msg = log4sp::FormatToAmxTplString(ctx, params, 2);
    }
    catch(const std::exception& e)
    {
        spdlog::source_loc loc = log4sp::GetScriptedLoc(ctx);
        logger->log(loc, spdlog::level::err, e.what());
        spdlog::log(loc, spdlog::level::err, e.what());
        return 0;
    }

    logger->info(msg);
    return 0;
}

/**
 * public native void Warn(const char[] msg);
 */
static cell_t Warn(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->warn(msg);
    return 0;
}

/**
 * public native void WarnAmxTpl(const char[] fmt, any ...);
 */
static cell_t WarnAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    std::string msg;
    try
    {
        msg = log4sp::FormatToAmxTplString(ctx, params, 2);
    }
    catch(const std::exception& e)
    {
        spdlog::source_loc loc = log4sp::GetScriptedLoc(ctx);
        logger->log(loc, spdlog::level::err, e.what());
        spdlog::log(loc, spdlog::level::err, e.what());
        return 0;
    }

    logger->warn(msg);
    return 0;
}

/**
 * public native void Error(const char[] msg);
 */
static cell_t Error(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->error(msg);
    return 0;
}

/**
 * public native void ErrorAmxTpl(const char[] fmt, any ...);
 */
static cell_t ErrorAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    std::string msg;
    try
    {
        msg = log4sp::FormatToAmxTplString(ctx, params, 2);
    }
    catch(const std::exception& e)
    {
        spdlog::source_loc loc = log4sp::GetScriptedLoc(ctx);
        logger->log(loc, spdlog::level::err, e.what());
        spdlog::log(loc, spdlog::level::err, e.what());
        return 0;
    }

    logger->error(msg);
    return 0;
}

/**
 * public native void Fatal(const char[] msg);
 */
static cell_t Fatal(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->critical(msg);
    return 0;
}

/**
 * public native void FatalAmxTpl(const char[] fmt, any ...);
 */
static cell_t FatalAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    std::string msg;
    try
    {
        msg = log4sp::FormatToAmxTplString(ctx, params, 2);
    }
    catch(const std::exception& e)
    {
        spdlog::source_loc loc = log4sp::GetScriptedLoc(ctx);
        logger->log(loc, spdlog::level::err, e.what());
        spdlog::log(loc, spdlog::level::err, e.what());
        return 0;
    }

    logger->critical(msg);
    return 0;
}

/**
 * public native void Flush();
 */
static cell_t Flush(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    logger->flush();
    return 0;
}

/**
 * public native LogLevel GetFlushLevel();
 */
static cell_t GetFlushLevel(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    return logger->flush_level();
}

/**
 * public native void FlushOn(LogLevel lvl);
 */
static cell_t FlushOn(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    auto lvl = static_cast<spdlog::level::level_enum>(params[2]);
    if (lvl < static_cast<spdlog::level::level_enum>(0))
    {
        lvl = static_cast<spdlog::level::level_enum>(0);
    }
    else if (lvl >= spdlog::level::n_levels)
    {
        lvl = static_cast<spdlog::level::level_enum>(spdlog::level::n_levels - 1);
    }

    logger->flush_on(lvl);
    return 0;
}

/**
 * Backtrace support: https://github.com/gabime/spdlog?tab=readme-ov-file#backtrace-support
 *
 * efficiently store all debug/trace messages in a circular buffer until needed for debugging.
 *
 * Debug messages can be stored in a ring buffer instead of being logged immediately.
 * This is useful to display debug logs only when needed (e.g. when an error happens).
 * When needed, call dump_backtrace() to dump them to your log.
 */
/**
 * public native bool ShouldBacktrace();
 */
static cell_t ShouldBacktrace(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    return logger->should_backtrace();
}

/**
 * public native void EnableBacktrace(int num);
 */
static cell_t EnableBacktrace(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    auto num = static_cast<size_t>(params[2]);

    logger->enable_backtrace(num);
    return 0;
}

/**
 * public native void DisableBacktrace();
 */
static cell_t DisableBacktrace(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    logger->disable_backtrace();
    return 0;
}

/**
 * public native void DumpBacktrace();
 */
static cell_t DumpBacktrace(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    logger->dump_backtrace();
    return 0;
}

/**
 * public native void AddSink(Sink sink);
 */
static cell_t AddSink(IPluginContext *ctx, const cell_t *params)
{
    auto loggerHandle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, loggerHandle);
    if (logger == nullptr)
    {
        return 0;
    }

    auto loggerData = log4sp::logger_handle_manager::instance().get_data(logger->name());
    if (loggerData == nullptr)
    {
        ctx->ReportError("Fatal internal error, logger data not found. (name='%s', hdl=%X)", logger->name(), static_cast<int>(loggerHandle));
        return 0;
    }

    auto sinkHandle = static_cast<Handle_t>(params[2]);
    auto sink = log4sp::sink_handle_manager::instance().read_handle(ctx, sinkHandle);
    if (sink == nullptr)
    {
        return 0;
    }

    auto sinkData = log4sp::sink_handle_manager::instance().get_data(sink);
    if (sinkData == nullptr)
    {
        ctx->ReportError("Fatal internal error, sink data not found. (hdl=%X)", static_cast<int>(sinkHandle));
        return 0;
    }

    bool async = loggerData->is_multi_threaded();
    if (async != sinkData->is_multi_threaded())
    {
        ctx->ReportError("You cannot add a %s sink to a %s logger.", async ? "synchronous" : "asynchronous", async ? "asynchronous" : "synchronous");
        return 0;
    }

    if (!async)
    {
        logger->sinks().push_back(sinkData->sink_ptr());
        return 0;
    }

    auto dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(logger->sinks().front());
    if (dist_sink == nullptr)
    {
        ctx->ReportError("Fatal internal error, cannot cast sink to dist_sink. (name='%s', hdl=%X)", logger->name(), static_cast<int>(loggerHandle));
        return 0;
    }

    dist_sink->add_sink(sinkData->sink_ptr());
    return 0;
}

/**
 * public native void DropSink(Sink sink);
 */
static cell_t DropSink(IPluginContext *ctx, const cell_t *params)
{
    auto loggerHandle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, loggerHandle);
    if (logger == nullptr)
    {
        return 0;
    }

    auto loggerData = log4sp::logger_handle_manager::instance().get_data(logger->name());
    if (loggerData == nullptr)
    {
        ctx->ReportError("Fatal internal error, logger data not found. (name='%s', hdl=%X)", logger->name(), static_cast<int>(loggerHandle));
        return 0;
    }

    auto sinkHandle = static_cast<Handle_t>(params[2]);
    auto sink = log4sp::sink_handle_manager::instance().read_handle(ctx, sinkHandle);
    if (sink == nullptr)
    {
        return 0;
    }

    auto sinkData = log4sp::sink_handle_manager::instance().get_data(sink);
    if (sinkData == nullptr)
    {
        ctx->ReportError("Fatal internal error, sink data not found. (hdl=%X)", static_cast<int>(sinkHandle));
        return 0;
    }

    if (!loggerData->is_multi_threaded())
    {
        auto iterator = std::find(logger->sinks().begin(), logger->sinks().end(), sinkData->sink_ptr());
        if (iterator == logger->sinks().end())
        {
            return false;
        }

        logger->sinks().erase(iterator);
        return true;
    }

    auto dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(logger->sinks().front());
    if (dist_sink == nullptr)
    {
        ctx->ReportError("Fatal internal error, cannot cast sink to dist_sink. (name='%s', hdl=%X)", logger->name(), static_cast<int>(loggerHandle));
        return 0;
    }

    dist_sink->remove_sink(sinkData->sink_ptr());
    return true;
}

/**
 * public native void SetErrorHandler(Log4spErrorCallback callback);
 *
 * function void (const char[] msg);
 */
static cell_t SetErrorHandler(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto logger = log4sp::logger_handle_manager::instance().read_handle(ctx, handle);
    if (logger == nullptr)
    {
        return 0;
    }

    auto data = log4sp::logger_handle_manager::instance().get_data(logger->name());
    if (data == nullptr)
    {
        ctx->ReportError("Fatal internal error, logger data not found. (name='%s', hdl=%X)", logger->name(), static_cast<int>(handle));
        return 0;
    }

    auto funcId = static_cast<funcid_t>(params[2]);
    auto callback = ctx->GetFunctionById(funcId);
    if (callback == NULL)
    {
        ctx->ReportError("Invalid function id (%X)", static_cast<int>(funcId));
        return 0;
    }

    auto forward = forwards->CreateForwardEx(NULL, ET_Ignore, 1, NULL, Param_String);
    if (forward == NULL)
    {
        ctx->ReportError("Could not create forward.");
        return 0;
    }

    if (!forward->AddFunction(callback))
    {
        forwards->ReleaseForward(forward);
        ctx->ReportError("Could not add callback.");
        return 0;
    }

    data->set_error_handler(forward);

    data->logger()->set_error_handler([forward](const std::string &msg) {
        forward->PushString(msg.c_str());
        forward->Execute();
    });
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
    {"Logger.LogAmxTpl",                        LogAmxTpl},
    {"Logger.LogSrc",                           LogSrc},
    {"Logger.LogSrcAmxTpl",                     LogSrcAmxTpl},
    {"Logger.LogLoc",                           LogLoc},
    {"Logger.LogLocAmxTpl",                     LogLocAmxTpl},
    {"Logger.LogStackTrace",                    LogStackTrace},
    {"Logger.LogStackTraceAmxTpl",              LogStackTraceAmxTpl},
    {"Logger.ThrowError",                       ThrowError},
    {"Logger.ThrowErrorAmxTpl",                 ThrowErrorAmxTpl},

    {"Logger.Trace",                            Trace},
    {"Logger.TraceAmxTpl",                      TraceAmxTpl},
    {"Logger.Debug",                            Debug},
    {"Logger.DebugAmxTpl",                      DebugAmxTpl},
    {"Logger.Info",                             Info},
    {"Logger.InfoAmxTpl",                       InfoAmxTpl},
    {"Logger.Warn",                             Warn},
    {"Logger.WarnAmxTpl",                       WarnAmxTpl},
    {"Logger.Error",                            Error},
    {"Logger.ErrorAmxTpl",                      ErrorAmxTpl},
    {"Logger.Fatal",                            Fatal},
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

    {NULL,                                      NULL}
};


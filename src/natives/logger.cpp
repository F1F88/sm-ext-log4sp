#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"

#include <log4sp/common.h>

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
 * public native Logger(const char[] name, Sink[] sinkArray, int num);
 */
static cell_t Logger(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (!log4sp::logger::CheckNameOrReportError(ctx, name))
    {
        return BAD_HANDLE;
    }

    cell_t *sinkArray;
    ctx->LocalToPhysAddr(params[2], &sinkArray);
    int num = params[3];

    std::vector<spdlog::sink_ptr> sinks;
    for (int i = 0; i < num; ++i)
    {
        spdlog::sink_ptr sink = log4sp::sinks::ReadHandleOrReportError(ctx, sinkArray[i]);
        if (sink == nullptr)
        {
            return BAD_HANDLE;
        }
        sinks.push_back(sink);
    }

    auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
    spdlog::register_logger(logger);    // 暂时不考虑匿名 logger
    return log4sp::logger::CreateHandleOrReportError(ctx, logger);
}

/**
 * public static native Logger Get(const char[] name);
 */
static cell_t Get(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (!log4sp::logger::CheckNameOrReportError(ctx, name))
    {
        return BAD_HANDLE;
    }

    auto logger = spdlog::get(name);
    return log4sp::logger::CreateHandleOrReportError(ctx, logger);
}

/**
 * public static native Logger CreateServerConsoleLogger(const char[] name, bool mt = false);
 */
static cell_t CreateServerConsoleLogger(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (!log4sp::logger::CheckNameOrReportError(ctx, name))
    {
        return BAD_HANDLE;
    }

    bool mt = params[2];

    auto logger = mt ?
        spdlog::stdout_logger_mt(name) :
        spdlog::stdout_logger_st(name);
    return log4sp::logger::CreateHandleOrReportError(ctx, logger);
}

/**
 * public static native Logger CreateBaseFileLogger(
 *     const char[] name,
 *     const char[] file,
 *     bool turncate = false,
 *     bool mt = false
 * );
 */
static cell_t CreateBaseFileLogger(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (!log4sp::logger::CheckNameOrReportError(ctx, name))
    {
        return BAD_HANDLE;
    }

    char *file;
    ctx->LocalToString(params[2], &file);
    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    bool turncate = params[3];
    bool mt = params[4];

    auto logger = mt ?
        spdlog::basic_logger_mt(name, path, turncate) :
        spdlog::basic_logger_st(name, path, turncate);
    return log4sp::logger::CreateHandleOrReportError(ctx, logger);
}

/**
 * public static native Logger CreateRotatingFileLogger(
 *      const char[] name,
 *      const char[] file,
 *      int maxFileSize,
 *      int maxFiles,
 *      bool rotateOnOpen = false,
 *      bool mt = false
 * );
 */
static cell_t CreateRotatingFileLogger(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (!log4sp::logger::CheckNameOrReportError(ctx, name))
    {
        return BAD_HANDLE;
    }

    char *file;
    ctx->LocalToString(params[2], &file);
    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    size_t maxFileSize = params[3];
    size_t maxFiles = params[4];
    bool rotateOnOpen = params[5];
    bool mt = params[6];

    auto logger = mt ?
        spdlog::rotating_logger_mt(name, path, maxFileSize, maxFiles, rotateOnOpen) :
        spdlog::rotating_logger_st(name, path, maxFileSize, maxFiles, rotateOnOpen);
    return log4sp::logger::CreateHandleOrReportError(ctx, logger);
}

/**
 * public static native Logger CreateDailyFileLogger(
 *      const char[] name,
 *      const char[] file,
 *      int hour = 0,
 *      int minute = 0,
 *      bool truncate = false,
 *      int maxFiles = 0,
 *      bool mt = false
 * );
 */
static cell_t CreateDailyFileLogger(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (!log4sp::logger::CheckNameOrReportError(ctx, name))
    {
        return BAD_HANDLE;
    }

    char *file;
    ctx->LocalToString(params[2], &file);
    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    int hour = params[3];
    int minute = params[4];
    bool truncate = params[5];
    uint16_t maxFiles = params[6];
    bool mt = params[7];

    auto logger = mt ?
        spdlog::daily_logger_format_mt(name, path, hour, minute, truncate, maxFiles) :
        spdlog::daily_logger_format_st(name, path, hour, minute, truncate, maxFiles);
    return log4sp::logger::CreateHandleOrReportError(ctx, logger);
}

/**
 * public native void GetName(char[] buffer, int maxlen);
 */
static cell_t GetName(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    ctx->StringToLocal(params[2], params[3], logger->name().c_str());
    return true;
}

/**
 * public native LogLevel GetLevel();
 */
static cell_t GetLevel(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    return logger->level();
}

/**
 * public native void SetLevel(LogLevel lvl);
 */
static cell_t SetLevel(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    spdlog::level::level_enum lvl = log4sp::CellToLevelOrLogWarn(ctx, params[2]);

    logger->set_level(lvl);
    return true;
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
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    char *pattern;
    ctx->LocalToString(params[2], &pattern);

    if (params[3] == 0)
    {
        logger->set_pattern(pattern, spdlog::pattern_time_type::local);
    }
    else
    {
        logger->set_pattern(pattern, spdlog::pattern_time_type::utc);
    }
    return true;
}

/**
 * public native bool ShouldLog(LogLevel lvl);
 */
static cell_t ShouldLog(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    spdlog::level::level_enum lvl = log4sp::CellToLevelOrLogWarn(ctx, params[2]);

    return logger->should_log(lvl);
}

/**
 * public native void Log(LogLevel lvl, const char[] msg);
 */
static cell_t Log(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    spdlog::level::level_enum lvl = log4sp::CellToLevelOrLogWarn(ctx, params[2]);
    char *msg;
    ctx->LocalToString(params[3], &msg);

    logger->log(lvl, msg);
    return true;
}

/**
 * public native void LogAmxTpl(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    spdlog::level::level_enum lvl = log4sp::CellToLevelOrLogWarn(ctx, params[2]);
    if (!logger->should_log(lvl))
    {
        return true;
    }

    char *msg = log4sp::FormatToAmxTplString(ctx, params, 3);

    logger->log(lvl, msg);
    return true;
}

/**
 * public native void LogSrc(LogLevel lvl, const char[] msg);
 */
static cell_t LogSrc(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    spdlog::level::level_enum lvl = log4sp::CellToLevelOrLogWarn(ctx, params[2]);
    char *msg;
    ctx->LocalToString(params[3], &msg);

    spdlog::source_loc loc = log4sp::GetScriptedLoc(ctx);
    logger->log(loc, lvl, msg);
    return true;
}

/**
 * public native void LogSrcAmxTpl(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogSrcAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    spdlog::level::level_enum lvl = log4sp::CellToLevelOrLogWarn(ctx, params[2]);
    if (!logger->should_log(lvl))
    {
        return true;
    }

    char *msg = log4sp::FormatToAmxTplString(ctx, params, 3);

    spdlog::source_loc loc = log4sp::GetScriptedLoc(ctx);
    logger->log(loc, lvl, msg);
    return true;
}

/**
 * public native void LogLoc(const char[] file, int line, const char[] func, LogLevel lvl, const char[] msg);
 */
static cell_t LogLoc(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    char *file, *func, *msg;
    ctx->LocalToString(params[2], &file);
    int line = params[3];
    ctx->LocalToString(params[4], &func);
    spdlog::level::level_enum lvl = log4sp::CellToLevelOrLogWarn(ctx, params[5]);
    ctx->LocalToString(params[6], &msg);

    spdlog::source_loc loc = {file, line, func};
    logger->log(loc, lvl, msg);
    return true;
}

/**
 * public native void LogLocAmxTpl(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogLocAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    char *file, *func;
    ctx->LocalToString(params[2], &file);
    int line = params[3];
    ctx->LocalToString(params[4], &func);
    spdlog::level::level_enum lvl = log4sp::CellToLevelOrLogWarn(ctx, params[5]);
    if (!logger->should_log(lvl))
    {
        return true;
    }

    char *msg = log4sp::FormatToAmxTplString(ctx, params, 6);

    spdlog::source_loc loc = {file, line, func};
    logger->log(loc, lvl, msg);
    return true;
}

/**
 * public native void Trace(const char[] msg);
 */
static cell_t Trace(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->trace(msg);
    return true;
}

/**
 * public native void TraceAmxTpl(const char[] fmt, any ...);
 */
static cell_t TraceAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    if (!logger->should_log(spdlog::level::trace))
    {
        return true;
    }

    char *msg = log4sp::FormatToAmxTplString(ctx, params, 2);

    logger->trace(msg);
    return true;
}

/**
 * public native void Debug(const char[] msg);
 */
static cell_t Debug(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->debug(msg);
    return true;
}

/**
 * public native void DebugAmxTpl(const char[] fmt, any ...);
 */
static cell_t DebugAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    if (!logger->should_log(spdlog::level::debug))
    {
        return true;
    }

    char *msg = log4sp::FormatToAmxTplString(ctx, params, 2);

    logger->debug(msg);
    return true;
}

/**
 * public native void Info(const char[] msg);
 */
static cell_t Info(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->info(msg);
    return true;
}

/**
 * public native void InfoAmxTpl(const char[] fmt, any ...);
 */
static cell_t InfoAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    if (!logger->should_log(spdlog::level::info))
    {
        return true;
    }

    char *msg = log4sp::FormatToAmxTplString(ctx, params, 2);

    logger->info(msg);
    return true;
}

/**
 * public native void Warn(const char[] msg);
 */
static cell_t Warn(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->warn(msg);
    return true;
}

/**
 * public native void WarnAmxTpl(const char[] fmt, any ...);
 */
static cell_t WarnAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    if (!logger->should_log(spdlog::level::warn))
    {
        return true;
    }

    char *msg = log4sp::FormatToAmxTplString(ctx, params, 2);

    logger->warn(msg);
    return true;
}

/**
 * public native void Error(const char[] msg);
 */
static cell_t Error(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->error(msg);
    return true;
}

/**
 * public native void ErrorAmxTpl(const char[] fmt, any ...);
 */
static cell_t ErrorAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    if (!logger->should_log(spdlog::level::err))
    {
        return true;
    }

    char *msg = log4sp::FormatToAmxTplString(ctx, params, 2);

    logger->error(msg);
    return true;
}

/**
 * public native void Fatal(const char[] msg);
 */
static cell_t Fatal(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->critical(msg);
    return true;
}

/**
 * public native void FatalAmxTpl(const char[] fmt, any ...);
 */
static cell_t FatalAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    if (!logger->should_log(spdlog::level::critical))
    {
        return true;
    }

    char *msg = log4sp::FormatToAmxTplString(ctx, params, 2);

    logger->critical(msg);
    return true;
}

/**
 * public native void Flush();
 */
static cell_t Flush(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    logger->flush();
    return true;
}

/**
 * public native LogLevel GetFlushLevel();
 */
static cell_t GetFlushLevel(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    return logger->flush_level();
}

/**
 * public native void FlushOn(LogLevel lvl);
 */
static cell_t FlushOn(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    spdlog::level::level_enum lvl = log4sp::CellToLevelOrLogWarn(ctx, params[2]);

    logger->flush_on(lvl);
    return true;
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
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    return logger->should_backtrace();
}

/**
 * public native void EnableBacktrace(int num);
 */
static cell_t EnableBacktrace(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    size_t num = params[2];

    logger->enable_backtrace(num);
    return true;
}

/**
 * public native void DisableBacktrace();
 */
static cell_t DisableBacktrace(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    logger->disable_backtrace();
    return true;
}

/**
 * public native void DumpBacktrace();
 */
static cell_t DumpBacktrace(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    logger->dump_backtrace();
    return true;
}

/**
 * public native void AddSink(Sink sink);
 */
static cell_t AddSink(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    spdlog::sink_ptr sink = log4sp::sinks::ReadHandleOrReportError(ctx, params[2]);
    if (sink == nullptr)
    {
        return false;
    }

    logger->sinks().push_back(sink);
    return true;
}

/**
 * public native bool DropSink(Sink sink);
 */
static cell_t DropSink(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    spdlog::sink_ptr sink = log4sp::sinks::ReadHandleOrReportError(ctx, params[2]);
    if (sink == nullptr)
    {
        return false;
    }

    auto iterator = std::find(logger->sinks().begin(), logger->sinks().end(), sink);
    if (iterator == logger->sinks().end())
    {
        return false;
    }

    logger->sinks().erase(iterator);
    return true;
}

/**
 * public native void SetErrorHandler(Log4spErrorCallback callback);
 *
 * function void (const char[] msg);
 */
static cell_t SetErrorHandler(IPluginContext *ctx, const cell_t *params)
{
    spdlog::logger *logger = log4sp::logger::ReadHandleOrReportError(ctx, params[1]);
    if (logger == nullptr)
    {
        return false;
    }

    IPluginFunction *callback = ctx->GetFunctionById(params[2]);
    if (callback == NULL )
    {
        ctx->ReportError("Invalid param callback %d.", params[2]);
        return false;
    }

    // TODO 也许可以优化为不需要每次都重复创建/释放forward
    logger->set_error_handler([callback](const std::string &msg) {
        IChangeableForward *forward = forwards->CreateForwardEx(NULL, ET_Ignore, 1, NULL, Param_String);
        if (forward == NULL || !forward->AddFunction(callback)) {
            return ;    // TODO 也许可以输出一些提示信息来提醒用户
        }

        forward->PushString(msg.c_str());
        forward->Execute();
        forwards->ReleaseForward(forward);
    });
    return true;
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
    {"Logger.Trace",                            Trace},
    {"Logger.TraceAmxTpl",                      TraceAmxTpl},
    {"Logger.Debug",                            Debug},
    {"Logger.DebugAmxTpl",                      DebugAmxTpl},
    {"Logger.Info",                             Info},
    {"Logger.InfoAmxTpl",                       InfoAmxTpl},
    {"Logger.Warn",                             Warn},
    {"Logger.WarnAmxTpl",                       WarnAmxTpl},
    {"Logger.Error",                            Error},
    {"Logger.ErrorAmxTpl",                      Error},
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


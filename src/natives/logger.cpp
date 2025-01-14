#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_sinks.h"

#include "log4sp/logger.h"
#include "log4sp/utils.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"


/**
 * public native Logger(const char[] name, Sink[] sinks, int numSinks);
 */
static cell_t Logger(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name) != BAD_HANDLE)
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    cell_t *sinks;
    ctx->LocalToPhysAddr(params[2], &sinks);

    auto numSinks = static_cast<size_t>(params[3]);
    std::vector<spdlog::sink_ptr> sinkVector(numSinks, nullptr);

    HandleSecurity security{ctx->GetIdentity(), myself->GetIdentity()};
    HandleError error;

    for (size_t i = 0; i < numSinks; ++i)
    {
        auto handle = static_cast<Handle_t>(sinks[i]);

        auto sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);
        if (sink == nullptr)
        {
            ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
            return BAD_HANDLE;
        }

        sinkVector[i] = sink;
    }

    auto logger = std::make_shared<log4sp::logger>(name, sinkVector.begin(), sinkVector.end());
    auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("SM error! Could not create logger handle (error: %d)", error);
        return BAD_HANDLE;
    }

    return handle;
}

/**
 * public static native Logger Get(const char[] name);
 */
static cell_t Get(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);

    return log4sp::logger_handler::instance().find_handle(name);
}

/**
 * public static native void ApplyAll(LoggerApplyCallback callback);
 *
 * function void (Logger logger, any data = 0);
 */
static cell_t ApplyAll(IPluginContext *ctx, const cell_t *params)
{
    auto funcID   = static_cast<funcid_t>(params[1]);
    auto function = ctx->GetFunctionById(funcID);
    if (function == nullptr)
    {
        ctx->ReportError("Invalid apply all function. (funcID: %d)", static_cast<int>(funcID));
        return 0;
    }

    IChangeableForward *forward = forwards->CreateForwardEx(nullptr, ET_Ignore, 2, nullptr, Param_Cell, Param_Cell);
    if (forward == nullptr)
    {
        ctx->ReportError("SM error! Create apply all forward failure.");
        return 0;
    }

    if (!forward->AddFunction(function))
    {
        forwards->ReleaseForward(forward);
        ctx->ReportError("SM error! Adding error handler function failed.");
        return 0;
    }

    auto data = params[2];

    log4sp::logger_handler::instance().apply_all(
        [forward, data](const Handle_t handle) {
            forward->PushCell(handle);
            forward->PushCell(data);
            forward->Execute();
        }
    );

    forwards->ReleaseForward(forward);
    return 0;
}

/**
 * public static native Logger CreateServerConsoleLogger(const char[] name);
 */
static cell_t CreateServerConsoleLogger(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name) != BAD_HANDLE)
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    spdlog::sink_ptr sink;
    try
    {
        sink = std::make_shared<spdlog::sinks::stdout_sink_st>();
    }
    catch(const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    HandleSecurity security{ctx->GetIdentity(), myself->GetIdentity()};
    HandleError error;

    auto logger = std::make_shared<log4sp::logger>(name, sink);
    auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("SM error! Could not create logger handle (error: %d)", error);
        return BAD_HANDLE;
    }

    return handle;
}

/**
 * public static native Logger CreateBaseFileLogger(const char[] name, const char[] file, bool truncate = false);
 */
static cell_t CreateBaseFileLogger(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name) != BAD_HANDLE)
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    char *file;
    ctx->LocalToString(params[2], &file);

    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto truncate = static_cast<bool>(params[3]);

    spdlog::sink_ptr sink;
    try
    {
        sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(path, truncate);
    }
    catch(const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    HandleSecurity security{ctx->GetIdentity(), myself->GetIdentity()};
    HandleError error;

    auto logger = std::make_shared<log4sp::logger>(name, sink);
    auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("SM error! Could not create logger handle (error: %d)", error);
        return BAD_HANDLE;
    }

    return handle;
}

/**
 * public static native Logger CreateRotatingFileLogger(const char[] name, const char[] file, int maxFileSize, int maxFiles, bool rotateOnOpen = false);
 */
static cell_t CreateRotatingFileLogger(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name) != BAD_HANDLE)
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    char *file;
    ctx->LocalToString(params[2], &file);

    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto maxFileSize  = static_cast<size_t>(params[3]);
    auto maxFiles     = static_cast<size_t>(params[4]);
    auto rotateOnOpen = static_cast<bool>(params[5]);

    spdlog::sink_ptr sink;
    try
    {
        sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(path, maxFileSize, maxFiles, rotateOnOpen);
    }
    catch(const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    HandleSecurity security{ctx->GetIdentity(), myself->GetIdentity()};
    HandleError error;

    auto logger = std::make_shared<log4sp::logger>(name, sink);
    auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("SM error! Could not create logger handle (error: %d)", error);
        return BAD_HANDLE;
    }

    return handle;
}

/**
 * public static native Logger CreateDailyFileLogger(const char[] name, const char[] file, int hour = 0, int minute = 0, bool truncate = false, int maxFiles = 0);
 */
static cell_t CreateDailyFileLogger(IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name) != BAD_HANDLE)
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    char *file;
    ctx->LocalToString(params[2], &file);

    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto hour     = static_cast<int>(params[3]);
    auto minute   = static_cast<int>(params[4]);
    auto truncate = static_cast<bool>(params[5]);
    auto maxFiles = static_cast<uint16_t>(params[6]);

    spdlog::sink_ptr sink;
    try
    {
        sink = std::make_shared<spdlog::sinks::daily_file_sink_st>(path, hour, minute, truncate, maxFiles);
    }
    catch(const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    HandleSecurity security{ctx->GetIdentity(), myself->GetIdentity()};
    HandleError error;

    auto logger = std::make_shared<log4sp::logger>(name, sink);
    auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("SM error! Could not create logger handle (error: %d)", error);
        return BAD_HANDLE;
    }

    return handle;
}

/**
 * public native int GetName(char[] buffer, int maxlen);
 */
static cell_t GetName(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{ctx->GetIdentity(), myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    size_t bytes;
    ctx->StringToLocalUTF8(params[2], params[3], logger->name().c_str(), &bytes);
    return bytes;
}

/**
 * public native int GetNameLength();
 */
static cell_t GetNameLength(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{ctx->GetIdentity(), myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    return logger->name().length();
}

/**
 * public native LogLevel GetLevel();
 */
static cell_t GetLevel(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    return static_cast<cell_t>(logger->level());
}

/**
 * public native void SetLevel(LogLevel lvl);
 */
static cell_t SetLevel(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    logger->set_level(lvl);
    return 0;
}

/**
 * public native void SetPattern(const char[] pattern, PatternTimeType type = PatternTimeType_local);
 */
static cell_t SetPattern(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    char *pattern;
    ctx->LocalToString(params[2], &pattern);

    auto type = log4sp::cell_to_pattern_time_type(params[3]);

    logger->set_pattern(pattern, type);
    return 0;
}

/**
 * public native bool ShouldLog(LogLevel lvl);
 */
static cell_t ShouldLog(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    return logger->should_log(lvl);
}

/**
 * public native void Log(LogLevel lvl, const char[] msg);
 */
static cell_t Log(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    char *msg;
    ctx->LocalToString(params[3], &msg);

    logger->log({}, lvl, msg, ctx);
    return 0;
}

/**
 * public native void LogEx(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    logger->log({}, lvl, ctx, params, 3);
    return 0;
}

/**
 * public native void LogAmxTpl(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    logger->log_amx_tpl({}, lvl, ctx, params, 3);
    return 0;
}

/**
 * public native void LogSrc(LogLevel lvl, const char[] msg);
 */
static cell_t LogSrc(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    char *msg;
    ctx->LocalToString(params[3], &msg);

    logger->log(log4sp::get_plugin_source_loc(ctx), lvl, msg, ctx);
    return 0;
}

/**
 * public native void LogSrcEx(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogSrcEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    logger->log(log4sp::get_plugin_source_loc(ctx), lvl, ctx, params, 3);
    return 0;
}

/**
 * public native void LogSrcAmxTpl(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogSrcAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    logger->log_amx_tpl({}, lvl, ctx, params, 3);
    return 0;
}

/**
 * public native void LogLoc(const char[] file, int line, const char[] func, LogLevel lvl, const char[] msg);
 */
static cell_t LogLoc(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    char *file, *func, *msg;
    ctx->LocalToString(params[2], &file);
    ctx->LocalToString(params[4], &func);
    ctx->LocalToString(params[6], &msg);

    auto line = static_cast<int>(params[3]);
    auto lvl  = log4sp::cell_to_level(params[5]);

    logger->log(spdlog::source_loc{file, line, func}, lvl, msg, ctx);
    return 0;
}

/**
 * public native void LogLocEx(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogLocEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    char *file, *func;
    ctx->LocalToString(params[2], &file);
    ctx->LocalToString(params[4], &func);

    auto line = static_cast<int>(params[3]);
    auto lvl  = log4sp::cell_to_level(params[5]);

    logger->log(spdlog::source_loc{file, line, func}, lvl, ctx, params, 6);
    return 0;
}

/**
 * public native void LogLocAmxTpl(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogLocAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    char *file, *func;
    ctx->LocalToString(params[2], &file);
    ctx->LocalToString(params[4], &func);

    auto line = static_cast<int>(params[3]);
    auto lvl  = log4sp::cell_to_level(params[5]);

    logger->log_amx_tpl(spdlog::source_loc{file, line, func}, lvl, ctx, params, 6);
    return 0;
}

/**
 * public native void LogStackTrace(LogLevel lvl, const char[] msg);
 */
static cell_t LogStackTrace(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    char *msg;
    ctx->LocalToString(params[3], &msg);

    logger->log_stack_trace(lvl, msg, ctx);
    return 0;
}

/**
 * public native void LogStackTraceEx(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogStackTraceEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    logger->log_stack_trace(lvl, ctx, params, 3);
    return 0;
}

/**
 * public native void LogStackTraceAmxTpl(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogStackTraceAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    logger->log_stack_trace(lvl, ctx, params, 3);
    return 0;
}

/**
 * public native void ThrowError(LogLevel lvl, const char[] msg);
 */
static cell_t ThrowError(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    char *msg;
    ctx->LocalToString(params[3], &msg);

    logger->throw_error(lvl, msg, ctx);
    return 0;
}

/**
 * public native void ThrowErrorEx(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t ThrowErrorEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    logger->throw_error(lvl, ctx, params, 3);
    return 0;
}

/**
 * public native void ThrowErrorAmxTpl(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t ThrowErrorAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    logger->throw_error_amx_tpl(lvl, ctx, params, 3);

    return 0;
}

/**
 * public native void Trace(const char[] msg);
 */
static cell_t Trace(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log({}, spdlog::level::trace, msg, ctx);
    return 0;
}

/**
 * public native void TraceEx(const char[] fmt, any ...);
 */
static cell_t TraceEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log({}, spdlog::level::trace, ctx, params, 2);
    return 0;
}

/**
 * public native void TraceAmxTpl(const char[] fmt, any ...);
 */
static cell_t TraceAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log_amx_tpl({}, spdlog::level::trace, ctx, params, 2);
    return 0;
}

/**
 * public native void Debug(const char[] msg);
 */
static cell_t Debug(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log({}, spdlog::level::debug, msg, ctx);
    return 0;
}

/**
 * public native void DebugEx(const char[] fmt, any ...);
 */
static cell_t DebugEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log({}, spdlog::level::debug, ctx, params, 2);
    return 0;
}

/**
 * public native void DebugAmxTpl(const char[] fmt, any ...);
 */
static cell_t DebugAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log_amx_tpl({}, spdlog::level::debug, ctx, params, 2);
    return 0;
}

/**
 * public native void Info(const char[] msg);
 */
static cell_t Info(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log({}, spdlog::level::info, msg, ctx);
    return 0;
}

/**
 * public native void InfoEx(const char[] fmt, any ...);
 */
static cell_t InfoEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log({}, spdlog::level::info, ctx, params, 2);
    return 0;
}

/**
 * public native void InfoAmxTpl(const char[] fmt, any ...);
 */
static cell_t InfoAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log_amx_tpl({}, spdlog::level::info, ctx, params, 2);
    return 0;
}

/**
 * public native void Warn(const char[] msg);
 */
static cell_t Warn(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log({}, spdlog::level::warn, msg, ctx);
    return 0;
}

/**
 * public native void WarnEx(const char[] fmt, any ...);
 */
static cell_t WarnEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log({}, spdlog::level::warn, ctx, params, 2);
    return 0;
}

/**
 * public native void WarnAmxTpl(const char[] fmt, any ...);
 */
static cell_t WarnAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log_amx_tpl({}, spdlog::level::warn, ctx, params, 2);
    return 0;
}

/**
 * public native void Error(const char[] msg);
 */
static cell_t Error(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log({}, spdlog::level::err, msg, ctx);
    return 0;
}

/**
 * public native void ErrorEx(const char[] fmt, any ...);
 */
static cell_t ErrorEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log({}, spdlog::level::err, ctx, params, 2);
    return 0;
}

/**
 * public native void ErrorAmxTpl(const char[] fmt, any ...);
 */
static cell_t ErrorAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log_amx_tpl({}, spdlog::level::err, ctx, params, 2);
    return 0;
}

/**
 * public native void Fatal(const char[] msg);
 */
static cell_t Fatal(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log({}, spdlog::level::critical, msg, ctx);
    return 0;
}

/**
 * public native void FatalEx(const char[] fmt, any ...);
 */
static cell_t FatalEx(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log({}, spdlog::level::critical, ctx, params, 2);
    return 0;
}

/**
 * public native void FatalAmxTpl(const char[] fmt, any ...);
 */
static cell_t FatalAmxTpl(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log_amx_tpl({}, spdlog::level::critical, ctx, params, 2);
    return 0;
}

/**
 * public native void Flush();
 */
static cell_t Flush(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    // TODO
    // logger->flush();
    return 0;
}

/**
 * public native LogLevel GetFlushLevel();
 */
static cell_t GetFlushLevel(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    return static_cast<cell_t>(logger->flush_level());
}

/**
 * public native void FlushOn(LogLevel lvl);
 */
static cell_t FlushOn(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    logger->flush_on(lvl);
    return 0;
}

/**
 * public native bool ShouldBacktrace();
 */
static cell_t ShouldBacktrace(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    return static_cast<cell_t>(logger->should_backtrace());
}

/**
 * public native void EnableBacktrace(int num);
 */
static cell_t EnableBacktrace(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
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

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
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

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
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

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(loggerHandle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", loggerHandle, error);
        return 0;
    }

    auto sinkHandle = static_cast<Handle_t>(params[2]);
    auto sink = log4sp::sink_handler::instance().read_handle(sinkHandle, &security, &error);
    if (sink == nullptr)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", sinkHandle, error);
        return 0;
    }

    logger->add_sink(sink);
    return 0;
}

/**
 * public native void AddSinkEx(Sink &sink);
 */
static cell_t AddSinkEx(IPluginContext *ctx, const cell_t *params)
{
    auto loggerHandle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(loggerHandle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", loggerHandle, error);
        return 0;
    }

    cell_t *sinkHandle;
    ctx->LocalToPhysAddr(params[2], &sinkHandle);
    auto sink = log4sp::sink_handler::instance().read_handle(*sinkHandle, &security, &error);
    if (sink == nullptr)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", sinkHandle, error);
        return 0;
    }

    logger->add_sink(sink);
    handlesys->FreeHandle(*sinkHandle, &security);
    *sinkHandle = BAD_HANDLE;
    return 0;
}

/**
 * public native void DropSink(Sink sink);
 */
static cell_t DropSink(IPluginContext *ctx, const cell_t *params)
{
    auto loggerHandle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(loggerHandle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", loggerHandle, error);
        return 0;
    }

    auto sinkHandle = static_cast<Handle_t>(params[2]);
    auto sink = log4sp::sink_handler::instance().read_handle(sinkHandle, &security, &error);
    if (sink == nullptr)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", sinkHandle, error);
        return 0;
    }

    logger->remove_sink(sink);
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

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (logger == nullptr)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto funcID   = static_cast<funcid_t>(params[2]);
    auto function = ctx->GetFunctionById(funcID);
    if (function == nullptr)
    {
        ctx->ReportError("Invalid error handler function id (%X)", static_cast<int>(funcID));
        return 0;
    }

    auto forward = forwards->CreateForwardEx(nullptr, ET_Ignore, 5, nullptr, Param_String, Param_String, Param_String, Param_Cell, Param_String);
    if (forward == nullptr)
    {
        ctx->ReportError("SM error! Could not create error handler forward.");
        return 0;
    }

    if (!forward->AddFunction(function))
    {
        forwards->ReleaseForward(forward);
        ctx->ReportError("SM error! Could not add error handler function.");
        return 0;
    }

    logger->set_error_handler(forward);
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
    {"Logger.ApplyAll",                         ApplyAll},

    {"Logger.GetName",                          GetName},
    {"Logger.GetNameLength",                    GetNameLength},
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
    {"Logger.AddSinkEx",                        AddSinkEx},
    {"Logger.DropSink",                         DropSink},
    {"Logger.SetErrorHandler",                  SetErrorHandler},

    {nullptr,                                   nullptr}
};


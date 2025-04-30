#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"

using spdlog::level::level_enum;
using spdlog::sink_ptr;


/**
 * public native Logger(const char[] name);
 */
static cell_t Logger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name) != BAD_HANDLE)
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security{ctx->GetIdentity(), myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = std::make_shared<log4sp::logger>(name);
    auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("SM error! Could not create logger handle (error: %d)", error);
        return BAD_HANDLE;
    }

    return handle;
}

/**
 * public native CreateLoggerWith(const char[] name, Sink[] sinks, int numSinks);
 */
static cell_t CreateLoggerWith(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
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

    auto numSinks = static_cast<int>(params[3]);
    auto sinkVector{std::vector<sink_ptr>(numSinks, nullptr)};

    SourceMod::HandleSecurity security{ctx->GetIdentity(), myself->GetIdentity()};
    SourceMod::HandleError error;

    for (int i = 0; i < numSinks; ++i)
    {
        auto sinkHandle = static_cast<SourceMod::Handle_t>(sinks[i]);

        auto sink = log4sp::sink_handler::instance().read_handle(sinkHandle, &security, &error);
        if (!sink)
        {
            ctx->ReportError("Invalid sink handle %x (error: %d)", sinkHandle, error);
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
 * public native CreateLoggerWithEx(const char[] name, Sink[] sinks, int numSinks);
 */
static cell_t CreateLoggerWithEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
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

    auto numSinks = static_cast<int>(params[3]);
    auto sinkVector{std::vector<sink_ptr>(numSinks, nullptr)};

    SourceMod::HandleSecurity security{ctx->GetIdentity(), myself->GetIdentity()};
    SourceMod::HandleError error;

    for (int i = 0; i < numSinks; ++i)
    {
        auto sinkHandle = static_cast<SourceMod::Handle_t>(sinks[i]);

        auto sink = log4sp::sink_handler::instance().read_handle(sinkHandle, &security, &error);
        if (!sink)
        {
            ctx->ReportError("Invalid sink handle %x (error: %d)", sinkHandle, error);
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

    for (int i = 0; i < numSinks; ++i)
    {
        auto sinkHandle = static_cast<SourceMod::Handle_t>(sinks[i]);
#ifndef DEBUG
        handlesys->FreeHandle(sinkHandle, &security);
#else
        assert(handlesys->FreeHandle(sinkHandle, &security) == SP_ERROR_NONE);
#endif
    }

    return handle;
}

/**
 * public static native Logger Get(const char[] name);
 */
static cell_t Get(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
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
static cell_t ApplyAll(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto funcid   = static_cast<funcid_t>(params[1]);
    auto function = ctx->GetFunctionById(funcid);
    if (!function)
    {
        ctx->ReportError("Invalid function id: 0x%08x", funcid);
        return 0;
    }

    auto forward = forwards->CreateForwardEx(nullptr, ET_Ignore, 2, nullptr, Param_Cell, Param_Cell);
    if (!forward)
    {
        ctx->ReportError("SM error! Create apply all forward failure.");
        return 0;
    }

    if (!forward->AddFunction(function))
    {
        forwards->ReleaseForward(forward);
        ctx->ReportError("SM error! Could not add apply all function.");
        return 0;
    }

    auto data = params[2];

    log4sp::logger_handler::instance().apply_all(
        [forward, data](const SourceMod::Handle_t handle) {
            forward->PushCell(handle);
            forward->PushCell(data);
#ifndef DEBUG
            forward->Execute();
#else
            assert(forward->Execute() == SP_ERROR_NONE);
#endif
        }
    );

    forwards->ReleaseForward(forward);
    return 0;
}

/**
 * public native int GetName(char[] buffer, int maxlen);
 */
static cell_t GetName(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    size_t bytes{0};
    ctx->StringToLocalUTF8(params[2], params[3], logger->name().c_str(), &bytes);
    return static_cast<cell_t>(bytes);
}

/**
 * public native int GetNameLength();
 */
static cell_t GetNameLength(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    return logger->name().length();
}

/**
 * public native LogLevel GetLevel();
 */
static cell_t GetLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    return static_cast<cell_t>(logger->level());
}

/**
 * public native void SetLevel(LogLevel lvl);
 */
static cell_t SetLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(static_cast<int>(params[2]));

    logger->set_level(lvl);
    return 0;
}

/**
 * public native void SetPattern(const char[] pattern, PatternTimeType type = PatternTimeType_local);
 */
static cell_t SetPattern(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    char *pattern;
    ctx->LocalToString(params[2], &pattern);

    auto type = log4sp::number_to_pattern_time_type(static_cast<int>(params[3]));

    logger->set_pattern(pattern, type);
    return 0;
}

/**
 * public native bool ShouldLog(LogLevel lvl);
 */
static cell_t ShouldLog(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(static_cast<int>(params[2]));

    return logger->should_log(lvl);
}

/**
 * public native void Log(LogLevel lvl, const char[] msg);
 */
static cell_t Log(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(static_cast<int>(params[2]));

    char *msg;
    ctx->LocalToString(params[3], &msg);

    logger->log({}, lvl, msg, ctx);
    return 0;
}

/**
 * public native void LogEx(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(static_cast<int>(params[2]));

    logger->log({}, lvl, ctx, params, 3);
    return 0;
}

/**
 * public native void LogAmxTpl(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(static_cast<int>(params[2]));

    logger->log_amx_tpl({}, lvl, ctx, params, 3);
    return 0;
}

/**
 * public native void LogSrc(LogLevel lvl, const char[] msg);
 */
static cell_t LogSrc(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(static_cast<int>(params[2]));

    char *msg;
    ctx->LocalToString(params[3], &msg);

    logger->log(log4sp::get_source_loc(ctx), lvl, msg, ctx);
    return 0;
}

/**
 * public native void LogSrcEx(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogSrcEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(static_cast<int>(params[2]));

    logger->log(log4sp::get_source_loc(ctx), lvl, ctx, params, 3);
    return 0;
}

/**
 * public native void LogSrcAmxTpl(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogSrcAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(static_cast<int>(params[2]));

    logger->log_amx_tpl(log4sp::get_source_loc(ctx), lvl, ctx, params, 3);
    return 0;
}

/**
 * public native void LogLoc(const char[] file, int line, const char[] func, LogLevel lvl, const char[] msg);
 */
static cell_t LogLoc(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    char *file, *func, *msg;
    ctx->LocalToString(params[2], &file);
    ctx->LocalToString(params[4], &func);
    ctx->LocalToString(params[6], &msg);

    auto line = static_cast<int>(params[3]);
    auto lvl = log4sp::num_to_lvl(static_cast<int>(params[5]));

    logger->log({file, line, func}, lvl, msg, ctx);
    return 0;
}

/**
 * public native void LogLocEx(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogLocEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    char *file, *func;
    ctx->LocalToString(params[2], &file);
    ctx->LocalToString(params[4], &func);

    auto line = static_cast<int>(params[3]);
    auto lvl  = log4sp::num_to_lvl(static_cast<int>(params[5]));

    logger->log({file, line, func}, lvl, ctx, params, 6);
    return 0;
}

/**
 * public native void LogLocAmxTpl(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogLocAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    char *file, *func;
    ctx->LocalToString(params[2], &file);
    ctx->LocalToString(params[4], &func);

    auto line = static_cast<int>(params[3]);
    auto lvl  = log4sp::num_to_lvl(static_cast<int>(params[5]));

    logger->log_amx_tpl({file, line, func}, lvl, ctx, params, 6);
    return 0;
}

/**
 * public native void LogStackTrace(LogLevel lvl, const char[] msg);
 */
static cell_t LogStackTrace(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(static_cast<int>(params[2]));

    char *msg;
    ctx->LocalToString(params[3], &msg);

    logger->log_stack_trace(lvl, msg, ctx);
    return 0;
}

/**
 * public native void LogStackTraceEx(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogStackTraceEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(static_cast<int>(params[2]));

    logger->log_stack_trace(lvl, ctx, params, 3);
    return 0;
}

/**
 * public native void LogStackTraceAmxTpl(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t LogStackTraceAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(static_cast<int>(params[2]));

    logger->log_stack_trace(lvl, ctx, params, 3);
    return 0;
}

/**
 * public native void ThrowError(LogLevel lvl, const char[] msg);
 */
static cell_t ThrowError(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(static_cast<int>(params[2]));

    char *msg;
    ctx->LocalToString(params[3], &msg);

    logger->throw_error(lvl, msg, ctx);
    return 0;
}

/**
 * public native void ThrowErrorEx(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t ThrowErrorEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(static_cast<int>(params[2]));

    logger->throw_error(lvl, ctx, params, 3);
    return 0;
}

/**
 * public native void ThrowErrorAmxTpl(LogLevel lvl, const char[] fmt, any ...);
 */
static cell_t ThrowErrorAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(static_cast<int>(params[2]));

    logger->throw_error_amx_tpl(lvl, ctx, params, 3);

    return 0;
}

/**
 * public native void Trace(const char[] msg);
 */
static cell_t Trace(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log({}, level_enum::trace, msg, ctx);
    return 0;
}

/**
 * public native void TraceEx(const char[] fmt, any ...);
 */
static cell_t TraceEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log({}, level_enum::trace, ctx, params, 2);
    return 0;
}

/**
 * public native void TraceAmxTpl(const char[] fmt, any ...);
 */
static cell_t TraceAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log_amx_tpl({}, level_enum::trace, ctx, params, 2);
    return 0;
}

/**
 * public native void Debug(const char[] msg);
 */
static cell_t Debug(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log({}, level_enum::debug, msg, ctx);
    return 0;
}

/**
 * public native void DebugEx(const char[] fmt, any ...);
 */
static cell_t DebugEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log({}, level_enum::debug, ctx, params, 2);
    return 0;
}

/**
 * public native void DebugAmxTpl(const char[] fmt, any ...);
 */
static cell_t DebugAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log_amx_tpl({}, level_enum::debug, ctx, params, 2);
    return 0;
}

/**
 * public native void Info(const char[] msg);
 */
static cell_t Info(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log({}, level_enum::info, msg, ctx);
    return 0;
}

/**
 * public native void InfoEx(const char[] fmt, any ...);
 */
static cell_t InfoEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log({}, level_enum::info, ctx, params, 2);
    return 0;
}

/**
 * public native void InfoAmxTpl(const char[] fmt, any ...);
 */
static cell_t InfoAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log_amx_tpl({}, level_enum::info, ctx, params, 2);
    return 0;
}

/**
 * public native void Warn(const char[] msg);
 */
static cell_t Warn(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log({}, level_enum::warn, msg, ctx);
    return 0;
}

/**
 * public native void WarnEx(const char[] fmt, any ...);
 */
static cell_t WarnEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log({}, level_enum::warn, ctx, params, 2);
    return 0;
}

/**
 * public native void WarnAmxTpl(const char[] fmt, any ...);
 */
static cell_t WarnAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log_amx_tpl({}, level_enum::warn, ctx, params, 2);
    return 0;
}

/**
 * public native void Error(const char[] msg);
 */
static cell_t Error(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log({}, level_enum::err, msg, ctx);
    return 0;
}

/**
 * public native void ErrorEx(const char[] fmt, any ...);
 */
static cell_t ErrorEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log({}, level_enum::err, ctx, params, 2);
    return 0;
}

/**
 * public native void ErrorAmxTpl(const char[] fmt, any ...);
 */
static cell_t ErrorAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log_amx_tpl({}, level_enum::err, ctx, params, 2);
    return 0;
}

/**
 * public native void Fatal(const char[] msg);
 */
static cell_t Fatal(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log({}, level_enum::critical, msg, ctx);
    return 0;
}

/**
 * public native void FatalEx(const char[] fmt, any ...);
 */
static cell_t FatalEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log({}, level_enum::critical, ctx, params, 2);
    return 0;
}

/**
 * public native void FatalAmxTpl(const char[] fmt, any ...);
 */
static cell_t FatalAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->log_amx_tpl({}, level_enum::critical, ctx, params, 2);
    return 0;
}

/**
 * public native void Flush();
 */
static cell_t Flush(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    logger->flush({}, ctx);
    return 0;
}

/**
 * public native LogLevel GetFlushLevel();
 */
static cell_t GetFlushLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    return static_cast<cell_t>(logger->flush_level());
}

/**
 * public native void FlushOn(LogLevel lvl);
 */
static cell_t FlushOn(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(static_cast<int>(params[2]));

    logger->flush_on(lvl);
    return 0;
}

/**
 * public native void AddSink(Sink sink);
 */
static cell_t AddSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto loggerHandle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(loggerHandle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", loggerHandle, error);
        return 0;
    }

    auto sinkHandle = static_cast<SourceMod::Handle_t>(params[2]);
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
 * public native void AddSinkEx(Sink sink);
 */
static cell_t AddSinkEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto loggerHandle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(loggerHandle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", loggerHandle, error);
        return 0;
    }

    auto sinkHandle = static_cast<SourceMod::Handle_t>(params[2]);
    auto sink = log4sp::sink_handler::instance().read_handle(sinkHandle, &security, &error);
    if (sink == nullptr)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", sinkHandle, error);
        return 0;
    }

    logger->add_sink(sink);
    handlesys->FreeHandle(sinkHandle, &security);
    return 0;
}

/**
 * public native void DropSink(Sink sink);
 */
static cell_t DropSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto loggerHandle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(loggerHandle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", loggerHandle, error);
        return 0;
    }

    auto sinkHandle = static_cast<SourceMod::Handle_t>(params[2]);
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
static cell_t SetErrorHandler(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);
        return 0;
    }

    auto funcid   = static_cast<funcid_t>(params[2]);
    auto function = ctx->GetFunctionById(funcid);
    if (!function)
    {
        ctx->ReportError("Invalid function id: 0x%08x", funcid);
        return 0;
    }

    auto forward = forwards->CreateForwardEx(nullptr, ET_Ignore, 5, nullptr, Param_String, Param_String, Param_String, Param_Cell, Param_String);
    if (!forward)
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
    {"Logger.CreateLoggerWith",                 CreateLoggerWith},
    {"Logger.CreateLoggerWithEx",               CreateLoggerWithEx},
    {"Logger.Get",                              Get},
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
    {"Logger.AddSink",                          AddSink},
    {"Logger.AddSinkEx",                        AddSinkEx},
    {"Logger.DropSink",                         DropSink},
    {"Logger.SetErrorHandler",                  SetErrorHandler},

    {nullptr,                                   nullptr}
};


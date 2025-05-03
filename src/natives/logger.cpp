#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"

using spdlog::level::level_enum;
using spdlog::sink_ptr;


static cell_t Logger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name))
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security{ctx->GetIdentity(), myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = std::make_shared<log4sp::logger>(name);
    if (auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error))
        return handle;

    ctx->ReportError("SM error! Could not create logger handle (error: %d)", error);
    return BAD_HANDLE;
}

static cell_t CreateLoggerWith(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name))
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    cell_t *sinks;
    ctx->LocalToPhysAddr(params[2], &sinks);

    int numSinks{params[3]};
    auto sinkVector{std::vector<sink_ptr>(numSinks, nullptr)};

    SourceMod::HandleSecurity security{ctx->GetIdentity(), myself->GetIdentity()};
    SourceMod::HandleError error;

    for (int i = 0; i < numSinks; ++i)
    {
        auto sink = log4sp::sink_handler::instance().read_handle(sinks[i], &security, &error);
        if (!sink)
        {
            ctx->ReportError("Invalid sink handle %x (error: %d)", sinks[i], error);
            return BAD_HANDLE;
        }

        sinkVector[i] = sink;
    }

    auto logger = std::make_shared<log4sp::logger>(name, sinkVector.begin(), sinkVector.end());
    if (auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error))
        return handle;

    ctx->ReportError("SM error! Could not create logger handle (error: %d)", error);
    return BAD_HANDLE;
}

static cell_t CreateLoggerWithEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name))
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    cell_t *sinks;
    ctx->LocalToPhysAddr(params[2], &sinks);

    int numSinks{params[3]};
    auto sinkVector{std::vector<sink_ptr>(numSinks, nullptr)};

    SourceMod::HandleSecurity security{ctx->GetIdentity(), myself->GetIdentity()};
    SourceMod::HandleError error;

    for (int i = 0; i < numSinks; ++i)
    {
        auto sink = log4sp::sink_handler::instance().read_handle(sinks[i], &security, &error);
        if (!sink)
        {
            ctx->ReportError("Invalid sink handle %x (error: %d)", sinks[i], error);
            return BAD_HANDLE;
        }

        sinkVector[i] = sink;
    }

    for (int i = 0; i < numSinks; ++i)
    {
#ifndef DEBUG
        handlesys->FreeHandle(sinks[i], &security);
#else
        assert(handlesys->FreeHandle(sinks[i], &security) == SP_ERROR_NONE);
#endif
    }

    auto logger = std::make_shared<log4sp::logger>(name, sinkVector.begin(), sinkVector.end());
    if (auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error))
        return handle;

    ctx->ReportError("SM error! Could not create logger handle (error: %d)", error);
    return BAD_HANDLE;
}

static cell_t Get(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    ctx->LocalToString(params[1], &name);

    return log4sp::logger_handler::instance().find_handle(name);
}

static cell_t ApplyAll(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto function = ctx->GetFunctionById(params[1]);
    if (!function)
    {
        ctx->ReportError("Invalid function id: 0x%08x", params[1]);
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

static cell_t GetName(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    size_t bytes{0};
    ctx->StringToLocalUTF8(params[2], params[3], logger->name().c_str(), &bytes);
    return static_cast<cell_t>(bytes);
}

static cell_t GetNameLength(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    return static_cast<cell_t>(logger->name().length());
}

static cell_t GetLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    return logger->level();
}

static cell_t SetLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(params[2]);

    logger->set_level(lvl);
    return 0;
}

static cell_t SetPattern(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    char *pattern;
    ctx->LocalToString(params[2], &pattern);

    auto type = log4sp::number_to_pattern_time_type(params[3]);

    logger->set_pattern(pattern, type);
    return 0;
}

static cell_t ShouldLog(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(params[2]);

    return logger->should_log(lvl);
}

static cell_t Log(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(params[2]);

    char *msg;
    ctx->LocalToString(params[3], &msg);

    logger->log({}, lvl, msg, ctx);
    return 0;
}

static cell_t LogEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(params[2]);

    logger->log({}, lvl, ctx, params, 3);
    return 0;
}

static cell_t LogAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(params[2]);

    logger->log_amx_tpl({}, lvl, ctx, params, 3);
    return 0;
}

static cell_t LogSrc(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(params[2]);

    char *msg;
    ctx->LocalToString(params[3], &msg);

    logger->log(log4sp::get_source_loc(ctx), lvl, msg, ctx);
    return 0;
}

static cell_t LogSrcEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(params[2]);

    logger->log(log4sp::get_source_loc(ctx), lvl, ctx, params, 3);
    return 0;
}

static cell_t LogSrcAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(params[2]);

    logger->log_amx_tpl(log4sp::get_source_loc(ctx), lvl, ctx, params, 3);
    return 0;
}

static cell_t LogLoc(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    char *file, *func, *msg;
    ctx->LocalToString(params[2], &file);
    ctx->LocalToString(params[4], &func);
    ctx->LocalToString(params[6], &msg);

    int line = params[3];
    auto lvl = log4sp::num_to_lvl(params[5]);

    logger->log({file, line, func}, lvl, msg, ctx);
    return 0;
}

static cell_t LogLocEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    char *file, *func;
    ctx->LocalToString(params[2], &file);
    ctx->LocalToString(params[4], &func);

    int line = params[3];
    auto lvl = log4sp::num_to_lvl(params[5]);

    logger->log({file, line, func}, lvl, ctx, params, 6);
    return 0;
}

static cell_t LogLocAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    char *file, *func;
    ctx->LocalToString(params[2], &file);
    ctx->LocalToString(params[4], &func);

    int line = params[3];
    auto lvl = log4sp::num_to_lvl(params[5]);

    logger->log_amx_tpl({file, line, func}, lvl, ctx, params, 6);
    return 0;
}

static cell_t LogStackTrace(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(params[2]);

    char *msg;
    ctx->LocalToString(params[3], &msg);

    logger->log_stack_trace(lvl, msg, ctx);
    return 0;
}

static cell_t LogStackTraceEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(params[2]);

    logger->log_stack_trace(lvl, ctx, params, 3);
    return 0;
}

static cell_t LogStackTraceAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(params[2]);

    logger->log_stack_trace(lvl, ctx, params, 3);
    return 0;
}

static cell_t ThrowError(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(params[2]);

    char *msg;
    ctx->LocalToString(params[3], &msg);

    logger->throw_error(lvl, msg, ctx);
    return 0;
}

static cell_t ThrowErrorEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(params[2]);

    logger->throw_error(lvl, ctx, params, 3);
    return 0;
}

static cell_t ThrowErrorAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(params[2]);

    logger->throw_error_amx_tpl(lvl, ctx, params, 3);

    return 0;
}

static cell_t Trace(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log({}, level_enum::trace, msg, ctx);
    return 0;
}

static cell_t TraceEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    logger->log({}, level_enum::trace, ctx, params, 2);
    return 0;
}

static cell_t TraceAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    logger->log_amx_tpl({}, level_enum::trace, ctx, params, 2);
    return 0;
}

static cell_t Debug(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log({}, level_enum::debug, msg, ctx);
    return 0;
}

static cell_t DebugEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    logger->log({}, level_enum::debug, ctx, params, 2);
    return 0;
}

static cell_t DebugAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    logger->log_amx_tpl({}, level_enum::debug, ctx, params, 2);
    return 0;
}

static cell_t Info(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log({}, level_enum::info, msg, ctx);
    return 0;
}

static cell_t InfoEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    logger->log({}, level_enum::info, ctx, params, 2);
    return 0;
}

static cell_t InfoAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    logger->log_amx_tpl({}, level_enum::info, ctx, params, 2);
    return 0;
}

static cell_t Warn(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log({}, level_enum::warn, msg, ctx);
    return 0;
}

static cell_t WarnEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    logger->log({}, level_enum::warn, ctx, params, 2);
    return 0;
}

static cell_t WarnAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    logger->log_amx_tpl({}, level_enum::warn, ctx, params, 2);
    return 0;
}

static cell_t Error(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log({}, level_enum::err, msg, ctx);
    return 0;
}

static cell_t ErrorEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    logger->log({}, level_enum::err, ctx, params, 2);
    return 0;
}

static cell_t ErrorAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    logger->log_amx_tpl({}, level_enum::err, ctx, params, 2);
    return 0;
}

static cell_t Fatal(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log({}, level_enum::critical, msg, ctx);
    return 0;
}

static cell_t FatalEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    logger->log({}, level_enum::critical, ctx, params, 2);
    return 0;
}

static cell_t FatalAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    logger->log_amx_tpl({}, level_enum::critical, ctx, params, 2);
    return 0;
}

static cell_t Flush(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    logger->flush({}, ctx);
    return 0;
}

static cell_t GetFlushLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    return logger->flush_level();
}

static cell_t FlushOn(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(params[2]);

    logger->flush_on(lvl);
    return 0;
}

static cell_t AddSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    if (auto sink = log4sp::sink_handler::instance().read_handle(params[2], &security, &error))
    {
        logger->add_sink(sink);
        return 0;
    }

    ctx->ReportError("Invalid sink handle %x (error: %d)", params[2], error);
    return 0;
}

static cell_t AddSinkEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    if (auto sink = log4sp::sink_handler::instance().read_handle(params[2], &security, &error))
    {
#ifndef DEBUG
        handlesys->FreeHandle(params[2], &security);
#else
        assert(handlesys->FreeHandle(params[2], &security) == SP_ERROR_NONE);
#endif
        logger->add_sink(sink);
        return 0;
    }

    ctx->ReportError("Invalid sink handle %x (error: %d)", params[2], error);
    return 0;
}

static cell_t DropSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    if (auto sink = log4sp::sink_handler::instance().read_handle(params[2], &security, &error))
    {
        logger->remove_sink(sink);
        return 0;
    }

    ctx->ReportError("Invalid sink handle %x (error: %d)", params[2], error);
    return 0;
}

static cell_t SetErrorHandler(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = log4sp::logger_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!logger)
    {
        ctx->ReportError("Invalid logger handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto function = ctx->GetFunctionById(params[2]);
    if (!function)
    {
        ctx->ReportError("Invalid function id: 0x%08x", params[2]);
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


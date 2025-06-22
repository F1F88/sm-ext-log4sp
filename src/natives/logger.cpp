#include "log4sp/common.h"
#include "log4sp/source_helper.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"

namespace fmt_lib = spdlog::fmt_lib;
using spdlog::level::level_enum;
using spdlog::sink_ptr;
using spdlog::source_loc;


/**
 * 封装读取 logger handle 代码
 * 这会创建 3 个变量: security, error, logger
 *      读取成功时: 继续执行后续代码
 *      读取失败时: 抛出错误并结束执行, 返回 0 (与 BAD_HANDLE 相同)
 */
#define READ_LOGGER_HANDLE_OR_ERROR(handle)                                                         \
    SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());                             \
    SourceMod::HandleError error;                                                                   \
    auto logger = log4sp::logger_handler::instance().read_handle_raw(handle, &security, &error);    \
    if (!logger) {                                                                                  \
        ctx->ReportError("Invalid logger handle %x (error: %d)", handle, error);                    \
        return 0;                                                                                   \
    }


static cell_t Logger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name))
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto logger = std::make_shared<log4sp::logger>(name);
    auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("SM error! Could not create logger handle (error: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
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
    std::vector<sink_ptr> sinkVector(numSinks, nullptr);

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
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
    auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("SM error! Could not create logger handle (error: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
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
    std::vector<sink_ptr> sinkVector(numSinks, nullptr);

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
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
    auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("SM error! Could not create logger handle (error: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
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
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    size_t bytes{0};
    ctx->StringToLocalUTF8(params[2], params[3], logger->name().c_str(), &bytes);
    return static_cast<cell_t>(bytes);
}

static cell_t GetNameLength(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    return static_cast<cell_t>(logger->name().length());
}

static cell_t GetLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    return logger->level();
}

static cell_t SetLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = log4sp::num_to_lvl(params[2]);

    logger->set_level(lvl);
    return 0;
}

static cell_t SetPattern(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *pattern;
    ctx->LocalToString(params[2], &pattern);

    auto type = log4sp::number_to_pattern_time_type(params[3]);

    logger->set_pattern(pattern, type);
    return 0;
}

static cell_t ShouldLog(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = log4sp::num_to_lvl(params[2]);

    return logger->should_log(lvl);
}

static cell_t Log(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = log4sp::num_to_lvl(params[2]);

    char *msg;
    ctx->LocalToString(params[3], &msg);

    logger->log(ctx, lvl, msg);
    return 0;
}

static cell_t LogEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = log4sp::num_to_lvl(params[2]);

    logger->log(ctx, lvl, params, 3);
    return 0;
}

static cell_t LogAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = log4sp::num_to_lvl(params[2]);

    logger->log_amx_tpl(ctx, lvl, params, 3);
    return 0;
}

static cell_t LogSrc(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = log4sp::num_to_lvl(params[2]);

    char *msg;
    ctx->LocalToString(params[3], &msg);

    logger->log(log4sp::src_helper::get_from_plugin_ctx(ctx), lvl, msg);
    return 0;
}

static cell_t LogSrcEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = log4sp::num_to_lvl(params[2]);

    logger->log(ctx, log4sp::src_helper::get_from_plugin_ctx(ctx), lvl, params, 3);
    return 0;
}

static cell_t LogSrcAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = log4sp::num_to_lvl(params[2]);

    logger->log_amx_tpl(ctx, log4sp::src_helper::get_from_plugin_ctx(ctx), lvl, params, 3);
    return 0;
}

static cell_t LogLoc(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *file, *func, *msg;
    ctx->LocalToString(params[2], &file);
    ctx->LocalToString(params[4], &func);
    ctx->LocalToString(params[6], &msg);

    int line = params[3];
    auto lvl = log4sp::num_to_lvl(params[5]);

    logger->log(source_loc(file, line, func), lvl, msg);
    return 0;
}

static cell_t LogLocEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *file, *func;
    ctx->LocalToString(params[2], &file);
    ctx->LocalToString(params[4], &func);

    int line = params[3];
    auto lvl = log4sp::num_to_lvl(params[5]);

    logger->log(ctx, source_loc(file, line, func), lvl, params, 6);
    return 0;
}

static cell_t LogLocAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *file, *func;
    ctx->LocalToString(params[2], &file);
    ctx->LocalToString(params[4], &func);

    int line = params[3];
    auto lvl = log4sp::num_to_lvl(params[5]);

    logger->log_amx_tpl(ctx, source_loc(file, line, func), lvl, params, 6);
    return 0;
}

static cell_t LogStackTrace(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = log4sp::num_to_lvl(params[2]);

    char *msg;
    ctx->LocalToString(params[3], &msg);

    logger->log(ctx, lvl, fmt_lib::format("Stack trace requested: {}", msg));
    logger->log(ctx, lvl, fmt_lib::format("Called from: {}", plsys->FindPluginByContext(ctx->GetContext())->GetFilename()));

    std::vector<std::string> messages = log4sp::src_helper::get_stack_trace(ctx);
    for (auto &iter : messages) {
        logger->log(ctx, lvl, iter);
    }
    return 0;
}

static cell_t LogStackTraceEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = log4sp::num_to_lvl(params[2]);

    logger->log_stack_trace(ctx, lvl, params, 3);
    return 0;
}

static cell_t LogStackTraceAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = log4sp::num_to_lvl(params[2]);

    logger->log_stack_trace_amx_tpl(ctx, lvl, params, 3);
    return 0;
}

static cell_t ThrowError(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = log4sp::num_to_lvl(params[2]);

    char *msg;
    ctx->LocalToString(params[3], &msg);

    ctx->ReportError(msg);

    logger->log(ctx, lvl, fmt_lib::format("Exception reported: {}", msg));
    logger->log(ctx, lvl, fmt_lib::format("Blaming: {}", plsys->FindPluginByContext(ctx->GetContext())->GetFilename()));

    std::vector<std::string> messages = log4sp::src_helper::get_stack_trace(ctx);
    for (auto &iter : messages) {
        logger->log(ctx, lvl, iter);
    }
    return 0;
}

static cell_t ThrowErrorEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = log4sp::num_to_lvl(params[2]);

    logger->throw_error(ctx, lvl, params, 3);
    return 0;
}

static cell_t ThrowErrorAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = log4sp::num_to_lvl(params[2]);

    logger->throw_error_amx_tpl(ctx, lvl, params, 3);
    return 0;
}

static cell_t Trace(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log(ctx, level_enum::trace, msg);
    return 0;
}

static cell_t TraceEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->log(ctx, level_enum::trace, params, 2);
    return 0;
}

static cell_t TraceAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->log_amx_tpl(ctx, level_enum::trace, params, 2);
    return 0;
}

static cell_t Debug(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log(ctx, level_enum::debug, msg);
    return 0;
}

static cell_t DebugEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->log(ctx, level_enum::debug, params, 2);
    return 0;
}

static cell_t DebugAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->log_amx_tpl(ctx, level_enum::debug, params, 2);
    return 0;
}

static cell_t Info(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log(ctx, level_enum::info, msg);
    return 0;
}

static cell_t InfoEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->log(ctx, level_enum::info, params, 2);
    return 0;
}

static cell_t InfoAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->log_amx_tpl(ctx, level_enum::info, params, 2);
    return 0;
}

static cell_t Warn(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log(ctx, level_enum::warn, msg);
    return 0;
}

static cell_t WarnEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->log(ctx, level_enum::warn, params, 2);
    return 0;
}

static cell_t WarnAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->log_amx_tpl(ctx, level_enum::warn, params, 2);
    return 0;
}

static cell_t Error(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log(ctx, level_enum::err, msg);
    return 0;
}

static cell_t ErrorEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->log(ctx, level_enum::err, params, 2);
    return 0;
}

static cell_t ErrorAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->log_amx_tpl(ctx, level_enum::err, params, 2);
    return 0;
}

static cell_t Fatal(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *msg;
    ctx->LocalToString(params[2], &msg);

    logger->log(ctx, level_enum::critical, msg);
    return 0;
}

static cell_t FatalEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->log(ctx, level_enum::critical, params, 2);
    return 0;
}

static cell_t FatalAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->log_amx_tpl(ctx, level_enum::critical, params, 2);
    return 0;
}

static cell_t Flush(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->flush(ctx);
    return 0;
}

static cell_t GetFlushLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    return logger->flush_level();
}

static cell_t FlushOn(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = log4sp::num_to_lvl(params[2]);

    logger->flush_on(lvl);
    return 0;
}

static cell_t AddSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto sink = log4sp::sink_handler::instance().read_handle(params[2], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", params[2], error);
        return 0;
    }

    logger->add_sink(sink);
    return 0;
}

static cell_t AddSinkEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto sink = log4sp::sink_handler::instance().read_handle(params[2], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", params[2], error);
        return 0;
    }

#ifndef DEBUG
    handlesys->FreeHandle(params[2], &security);
#else
    assert(handlesys->FreeHandle(params[2], &security) == SP_ERROR_NONE);
#endif

    logger->add_sink(sink);
    return 0;
}

static cell_t DropSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto sink = log4sp::sink_handler::instance().read_handle(params[2], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", params[2], error);
        return 0;
    }

    logger->remove_sink(sink);
    return 0;
}

static cell_t SetErrorHandler(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

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


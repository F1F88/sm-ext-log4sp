#include "log4sp/common.h"
#include "log4sp/source_helper.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_handler.h"


/**
 * 封装读取 logger handle 代码
 * 这会创建 1 个变量: logger
 *      读取成功时: 继续执行后续代码
 *      读取失败时: 抛出错误并结束执行, 返回 0 (与 BAD_HANDLE 相同)
 */
#define READ_LOGGER_HANDLE_OR_ERROR(handle)                                                         \
    Log4sp::Logger *logger;                                                                         \
    {                                                                                               \
        SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());                         \
        SourceMod::HandleError error;                                                               \
        logger = Log4sp::LoggerHandler::Instance().ReadHandleRaw(handle, &security, &error);        \
        if (!logger)                                                                                \
        {                                                                                           \
            ctx->ReportError("Invalid Logger Handle %x (error code: %d)", handle, error);           \
            return 0;                                                                               \
        }                                                                                           \
    }


static cell_t Logger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    CTX_LOCAL_TO_STRING(params[1], &name);
    if (Log4sp::LoggerHandler::Instance().FindHandle(name))
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto logger = std::make_shared<Log4sp::Logger>(name);
    auto handle = Log4sp::LoggerHandler::Instance().CreateHandle(logger, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("Failed to creates a Logger Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t CreateLoggerWith(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    CTX_LOCAL_TO_STRING(params[1], &name);
    if (Log4sp::LoggerHandler::Instance().FindHandle(name))
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    cell_t *sinks;
    CTX_LOCAL_TO_PHYS_ADDR(params[2], &sinks);

    using spdlog::sink_ptr;
    int numSinks = params[3];
    std::vector<sink_ptr> sinkVector(numSinks, nullptr);

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    for (int i = 0; i < numSinks; ++i)
    {
        auto sink = Log4sp::SinkHandler::Instance().ReadHandle(sinks[i], &security, &error);
        if (!sink)
        {
            ctx->ReportError("Invalid Sink Handle %x (index: %d, error code: %d)", sinks[i], i, error);
            return BAD_HANDLE;
        }

        sinkVector[i] = sink;
    }

    auto logger = std::make_shared<Log4sp::Logger>(name, sinkVector.begin(), sinkVector.end());
    auto handle = Log4sp::LoggerHandler::Instance().CreateHandle(logger, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("Failed to creates a Logger Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t CreateLoggerWithEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    CTX_LOCAL_TO_STRING(params[1], &name);
    if (Log4sp::LoggerHandler::Instance().FindHandle(name))
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    cell_t *sinks;
    CTX_LOCAL_TO_PHYS_ADDR(params[2], &sinks);

    using spdlog::sink_ptr;
    int numSinks = params[3];
    std::vector<sink_ptr> sinkVector(numSinks, nullptr);

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    for (int i = 0; i < numSinks; ++i)
    {
        auto sink = Log4sp::SinkHandler::Instance().ReadHandle(sinks[i], &security, &error);
        if (!sink)
        {
            ctx->ReportError("Invalid Sink Handle %x (index: %d, error code: %d)", sinks[i], i, error);
            return BAD_HANDLE;
        }

        sinkVector[i] = sink;
    }

    for (int i = 0; i < numSinks; ++i)
    {
        HANDLE_SYS_FREE_HANDLE(sinks[i], &security);
    }

    auto logger = std::make_shared<Log4sp::Logger>(name, sinkVector.begin(), sinkVector.end());
    auto handle = Log4sp::LoggerHandler::Instance().CreateHandle(logger, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("Failed to creates a Logger Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t Get(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    CTX_LOCAL_TO_STRING(params[1], &name);

    return Log4sp::LoggerHandler::Instance().FindHandle(name);
}

static cell_t ApplyAll(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto func = ctx->GetFunctionById(params[1]);
    if (!func)
    {
        ctx->ReportError("Invalid function id: 0x%08x.", params[1]);
        return 0;
    }

    // void (Logger logger, any data)
    FWDS_CREATE_EX(nullptr, ET_Ignore, 2, nullptr, Param_Cell, Param_Cell);
    FWD_ADD_FUNCTION(func);

    auto data = params[2];

    Log4sp::LoggerHandler::Instance().ApplyAll(
        [fwd, data](const SourceMod::Handle_t handle) {
            FWD_PUSH_CELL(handle);
            FWD_PUSH_CELL(data);
            FWD_EXECUTE();
        }
    );

    forwards->ReleaseForward(fwd);
    return 0;
}

static cell_t GetName(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    std::size_t bytes = 0;
    CTX_STRING_TO_LOCAL_UTF8(params[2], params[3], logger->Name().c_str(), &bytes);
    return static_cast<cell_t>(bytes);
}

static cell_t GetNameLength(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    return static_cast<cell_t>(logger->Name().length());
}

static cell_t GetLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    return logger->GetLevel();
}

static cell_t SetLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    logger->SetLevel(lvl);
    return 0;
}

static cell_t SetPattern(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *pattern;
    CTX_LOCAL_TO_STRING(params[2], &pattern);

    auto type = Log4sp::NumToPatternTimeType(params[3]);

    logger->SetPattern(pattern, type);
    return 0;
}

static cell_t ShouldLog(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    return logger->ShouldLog(lvl);
}

static cell_t Log(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    char *msg;
    CTX_LOCAL_TO_STRING(params[3], &msg);

    logger->Log(ctx, lvl, msg);
    return 0;
}

static cell_t LogEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    logger->Log(ctx, lvl, params, 3);
    return 0;
}

static cell_t LogAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    logger->LogAmxTpl(ctx, lvl, params, 3);
    return 0;
}

static cell_t LogSrc(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    char *msg;
    CTX_LOCAL_TO_STRING(params[3], &msg);

    logger->Log(Log4sp::SrcHelper::GetFromPluginCtx(ctx), lvl, msg);
    return 0;
}

static cell_t LogSrcEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    logger->Log(ctx, Log4sp::SrcHelper::GetFromPluginCtx(ctx), lvl, params, 3);
    return 0;
}

static cell_t LogSrcAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    logger->LogAmxTpl(ctx, Log4sp::SrcHelper::GetFromPluginCtx(ctx), lvl, params, 3);
    return 0;
}

static cell_t LogLoc(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *file, *func, *msg;
    CTX_LOCAL_TO_STRING(params[2], &file);
    CTX_LOCAL_TO_STRING(params[4], &func);
    CTX_LOCAL_TO_STRING(params[6], &msg);

    int line = params[3];
    auto lvl = Log4sp::NumToLvl(params[5]);

    logger->Log(spdlog::source_loc(file, line, func), lvl, msg);
    return 0;
}

static cell_t LogLocEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *file, *func;
    CTX_LOCAL_TO_STRING(params[2], &file);
    CTX_LOCAL_TO_STRING(params[4], &func);

    int line = params[3];
    auto lvl = Log4sp::NumToLvl(params[5]);

    logger->Log(ctx, spdlog::source_loc(file, line, func), lvl, params, 6);
    return 0;
}

static cell_t LogLocAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *file, *func;
    CTX_LOCAL_TO_STRING(params[2], &file);
    CTX_LOCAL_TO_STRING(params[4], &func);

    int line = params[3];
    auto lvl = Log4sp::NumToLvl(params[5]);

    logger->LogAmxTpl(ctx, spdlog::source_loc(file, line, func), lvl, params, 6);
    return 0;
}

static cell_t LogStackTrace(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    char *msg;
    CTX_LOCAL_TO_STRING(params[3], &msg);

    using spdlog::fmt_lib::format;
    logger->Log(ctx, lvl, format("Stack trace requested: {}", msg));
    logger->Log(ctx, lvl, format("Called from: {}", Log4sp::PluginSysFindPluginByCtx(ctx)->GetFilename()));

    std::vector<std::string> messages = Log4sp::SrcHelper::GetStackTrace(ctx);
    for (auto &iter : messages)
    {
        logger->Log(ctx, lvl, iter);
    }
    return 0;
}

static cell_t LogStackTraceEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    logger->LogStackTrace(ctx, lvl, params, 3);
    return 0;
}

static cell_t LogStackTraceAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    logger->LogStackTraceAmxTpl(ctx, lvl, params, 3);
    return 0;
}

static cell_t ThrowError(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    char *msg;
    CTX_LOCAL_TO_STRING(params[3], &msg);

    ctx->ReportError(msg);

    using spdlog::fmt_lib::format;
    logger->Log(ctx, lvl, format("Exception reported: {}", msg));
    logger->Log(ctx, lvl, format("Blaming: {}", Log4sp::PluginSysFindPluginByCtx(ctx)->GetFilename()));

    std::vector<std::string> messages = Log4sp::SrcHelper::GetStackTrace(ctx);
    for (auto &iter : messages)
    {
        logger->Log(ctx, lvl, iter);
    }
    return 0;
}

static cell_t ThrowErrorEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    logger->ThrowError(ctx, lvl, params, 3);
    return 0;
}

static cell_t ThrowErrorAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    logger->ThrowErrorAmxTpl(ctx, lvl, params, 3);
    return 0;
}

static cell_t Trace(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using spdlog::level::level_enum;
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *msg;
    CTX_LOCAL_TO_STRING(params[2], &msg);

    logger->Log(ctx, level_enum::trace, msg);
    return 0;
}

static cell_t TraceEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using spdlog::level::level_enum;
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->Log(ctx, level_enum::trace, params, 2);
    return 0;
}

static cell_t TraceAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using spdlog::level::level_enum;
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->LogAmxTpl(ctx, level_enum::trace, params, 2);
    return 0;
}

static cell_t Debug(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using spdlog::level::level_enum;
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *msg;
    CTX_LOCAL_TO_STRING(params[2], &msg);

    logger->Log(ctx, level_enum::debug, msg);
    return 0;
}

static cell_t DebugEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using spdlog::level::level_enum;
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->Log(ctx, level_enum::debug, params, 2);
    return 0;
}

static cell_t DebugAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using spdlog::level::level_enum;
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->LogAmxTpl(ctx, level_enum::debug, params, 2);
    return 0;
}

static cell_t Info(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using spdlog::level::level_enum;
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *msg;
    CTX_LOCAL_TO_STRING(params[2], &msg);

    logger->Log(ctx, level_enum::info, msg);
    return 0;
}

static cell_t InfoEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using spdlog::level::level_enum;
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->Log(ctx, level_enum::info, params, 2);
    return 0;
}

static cell_t InfoAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using spdlog::level::level_enum;
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->LogAmxTpl(ctx, level_enum::info, params, 2);
    return 0;
}

static cell_t Warn(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using spdlog::level::level_enum;
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *msg;
    CTX_LOCAL_TO_STRING(params[2], &msg);

    logger->Log(ctx, level_enum::warn, msg);
    return 0;
}

static cell_t WarnEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using spdlog::level::level_enum;
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->Log(ctx, level_enum::warn, params, 2);
    return 0;
}

static cell_t WarnAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using spdlog::level::level_enum;
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->LogAmxTpl(ctx, level_enum::warn, params, 2);
    return 0;
}

static cell_t Error(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using spdlog::level::level_enum;
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *msg;
    CTX_LOCAL_TO_STRING(params[2], &msg);

    logger->Log(ctx, level_enum::err, msg);
    return 0;
}

static cell_t ErrorEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using spdlog::level::level_enum;
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->Log(ctx, level_enum::err, params, 2);
    return 0;
}

static cell_t ErrorAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using spdlog::level::level_enum;
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->LogAmxTpl(ctx, level_enum::err, params, 2);
    return 0;
}

static cell_t Fatal(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using spdlog::level::level_enum;
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *msg;
    CTX_LOCAL_TO_STRING(params[2], &msg);

    logger->Log(ctx, level_enum::critical, msg);
    return 0;
}

static cell_t FatalEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using spdlog::level::level_enum;
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->Log(ctx, level_enum::critical, params, 2);
    return 0;
}

static cell_t FatalAmxTpl(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using spdlog::level::level_enum;
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->LogAmxTpl(ctx, level_enum::critical, params, 2);
    return 0;
}

static cell_t Flush(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->Flush(ctx);
    return 0;
}

static cell_t GetFlushLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    return logger->GetFlushLevel();
}

static cell_t FlushOn(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    logger->SetFlushLevel(lvl);
    return 0;
}

static cell_t AddSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());
    SourceMod::HandleError error;
    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[2], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error code: %d)", params[2], error);
        return 0;
    }

    logger->AddSink(sink);
    return 0;
}

static cell_t AddSinkEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[2], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error code: %d)", params[2], error);
        return 0;
    }

    HANDLE_SYS_FREE_HANDLE(params[2], &security);

    logger->AddSink(sink);
    return 0;
}

static cell_t DropSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[2], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error code: %d)", params[2], error);
        return 0;
    }

    logger->DropSink(sink);
    return 0;
}

static cell_t SetErrorHandler(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto func = ctx->GetFunctionById(params[2]);
    if (!func)
    {
        ctx->ReportError("Invalid function id: 0x%08x.", params[2]);
        return 0;
    }

    // void (const char[] msg, const char[] name, const char[] file, int line, const char[] func)
    FWDS_CREATE_EX(nullptr, ET_Ignore, 5, nullptr, Param_String, Param_String, Param_String, Param_Cell, Param_String);
    FWD_ADD_FUNCTION(func);

    logger->SetErrorHandler(fwd);
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


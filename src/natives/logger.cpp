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

static cell_t LogLoc(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    Log4sp::CellSourceLoc *loc;
    if (auto err = ctx->LocalToPhysAddr(params[2], reinterpret_cast<cell_t**>(&loc)))
    {
        ctx->ReportError("Invalid loc (error %d)", err);
        return 0;
    }

    auto lvl = Log4sp::NumToLvl(params[3]);

    char *msg;
    CTX_LOCAL_TO_STRING(params[4], &msg);

    logger->Log(loc->ToSourceLoc(), lvl, msg);
    return 0;
}

static cell_t LogLocEx(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    Log4sp::CellSourceLoc *loc;
    if (auto err = ctx->LocalToPhysAddr(params[2], reinterpret_cast<cell_t**>(&loc)))
    {
        ctx->ReportError("Invalid loc (error %d)", err);
        return 0;
    }

    auto lvl = Log4sp::NumToLvl(params[3]);

    logger->Log(ctx, loc->ToSourceLoc(), lvl, params, 4);
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

    SourceMod::IPlugin *plugin;
    if (!params[2])
    {
        plugin = Log4sp::PluginSysFindPluginByCtx(ctx);
    }
    else
    {
        SourceMod::HandleError error;
        plugin = plsys->PluginFromHandle(params[2], &error);
        if (!plugin)
        {
            ctx->ReportError("Invalid Plugin Handle %x (error %d)", params[2], error);
            return 0;
        }
    }

    SourcePawn::IPluginFunction *func = plugin->GetBaseContext()->GetFunctionById(params[3]);
    if (!func)
    {
        ctx->ReportError("Invalid function id %x.", params[3]);
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
    {"Logger.GetName",                          GetName},
    {"Logger.GetNameLength",                    GetNameLength},
    {"Logger.GetLevel",                         GetLevel},
    {"Logger.SetLevel",                         SetLevel},
    {"Logger.SetPattern",                       SetPattern},
    {"Logger.ShouldLog",                        ShouldLog},

    {"Logger.Log",                              Log},
    {"Logger.LogEx",                            LogEx},
    {"Logger.LogSrc",                           LogSrc},
    {"Logger.LogSrcEx",                         LogSrcEx},
    {"Logger.LogLoc",                           LogLoc},
    {"Logger.LogLocEx",                         LogLocEx},
    {"Logger.LogStackTrace",                    LogStackTrace},
    {"Logger.LogStackTraceEx",                  LogStackTraceEx},

    {"Logger.Trace",                            Trace},
    {"Logger.TraceEx",                          TraceEx},
    {"Logger.Debug",                            Debug},
    {"Logger.DebugEx",                          DebugEx},
    {"Logger.Info",                             Info},
    {"Logger.InfoEx",                           InfoEx},
    {"Logger.Warn",                             Warn},
    {"Logger.WarnEx",                           WarnEx},
    {"Logger.Error",                            Error},
    {"Logger.ErrorEx",                          ErrorEx},
    {"Logger.Fatal",                            Fatal},
    {"Logger.FatalEx",                          FatalEx},

    {"Logger.Flush",                            Flush},
    {"Logger.GetFlushLevel",                    GetFlushLevel},
    {"Logger.FlushOn",                          FlushOn},
    {"Logger.AddSink",                          AddSink},
    {"Logger.DropSink",                         DropSink},
    {"Logger.SetErrorHandler",                  SetErrorHandler},

    {nullptr,                                   nullptr}
};


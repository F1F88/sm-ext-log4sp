#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_handler.h"
#include "log4sp/sinks/callback_sink.h"

/**
 * 封装读取 logger handle 代码
 * 这会创建 1 个变量: logger
 *      读取成功时: 继续执行后续代码
 *      读取失败时: 抛出错误并结束执行, 返回 0 (与 BAD_HANDLE 相同)
 */
#define READ_LOGGER_HANDLE_OR_ERROR(handle)                                                         \
    Log4sp::Logger *logger;                                                                         \
    {                                                                                               \
        SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());              \
        SourceMod::HandleError error;                                                               \
        logger = Log4sp::LoggerHandler::Instance().ReadHandle(handle, &security, &error);           \
        if (!logger)                                                                                \
        {                                                                                           \
            ctx->ReportError("Invalid Logger Handle %x (error %d)", handle, error);                 \
            return 0;                                                                               \
        }                                                                                           \
    }


static cell_t Logger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    if (auto err = ctx->LocalToStringNULL(params[1], &name))
    {
        ctx->ReportError("Invalid name (error %d)", err);
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto logger = new Log4sp::Logger(name ? name : Log4sp::PluginSysFindPluginByCtx(ctx)->GetFilename());
    auto handle = Log4sp::LoggerHandler::Instance().CreateHandle(logger, &security, nullptr, &error);
    if (!handle)
    {
        delete logger;
        ctx->ReportError("Failed to creates a Logger Handle (error %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t Log(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    char *msg;
    if (auto err = ctx->LocalToString(params[3], &msg))
    {
        ctx->ReportError("Invalid msg (error %d)", err);
        return 0;
    }

    logger->Log(ctx, lvl, msg);
    return 0;
}

static cell_t LogF(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
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
    if (auto err = ctx->LocalToString(params[3], &msg))
    {
        ctx->ReportError("Invalid msg (error %d)", err);
        return 0;
    }

    auto loc = Log4sp::SourceLocFrom(ctx);

    logger->Log(ctx, loc, lvl, msg);
    return 0;
}

static cell_t LogSrcF(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    logger->Log(ctx, Log4sp::SourceLocFrom(ctx), lvl, params, 3);
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
    if (auto err = ctx->LocalToString(params[4], &msg))
    {
        ctx->ReportError("Invalid msg (error %d)", err);
        return 0;
    }

    logger->Log(ctx, loc->ToSourceLoc(), lvl, msg);
    return 0;
}

static cell_t LogLocF(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
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
    if (auto err = ctx->LocalToString(params[3], &msg))
    {
        ctx->ReportError("Invalid msg (error %d)", err);
        return 0;
    }

    logger->LogStackTrace(ctx, lvl, msg);
    return 0;
}

static cell_t LogStackTraceF(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
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
    if (auto err = ctx->LocalToString(params[2], &msg))
    {
        ctx->ReportError("Invalid msg (error %d)", err);
        return 0;
    }

    logger->Log(ctx, level_enum::trace, msg);
    return 0;
}

static cell_t TraceF(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
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
    if (auto err = ctx->LocalToString(params[2], &msg))
    {
        ctx->ReportError("Invalid msg (error %d)", err);
        return 0;
    }

    logger->Log(ctx, level_enum::debug, msg);
    return 0;
}

static cell_t DebugF(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
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
    if (auto err = ctx->LocalToString(params[2], &msg))
    {
        ctx->ReportError("Invalid msg (error %d)", err);
        return 0;
    }

    logger->Log(ctx, level_enum::info, msg);
    return 0;
}

static cell_t InfoF(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
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
    if (auto err = ctx->LocalToString(params[2], &msg))
    {
        ctx->ReportError("Invalid msg (error %d)", err);
        return 0;
    }

    logger->Log(ctx, level_enum::warn, msg);
    return 0;
}

static cell_t WarnF(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
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
    if (auto err = ctx->LocalToString(params[2], &msg))
    {
        ctx->ReportError("Invalid msg (error %d)", err);
        return 0;
    }

    logger->Log(ctx, level_enum::err, msg);
    return 0;
}

static cell_t ErrorF(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
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
    if (auto err = ctx->LocalToString(params[2], &msg))
    {
        ctx->ReportError("Invalid msg (error %d)", err);
        return 0;
    }

    logger->Log(ctx, level_enum::critical, msg);
    return 0;
}

static cell_t FatalF(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using spdlog::level::level_enum;
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    logger->Log(ctx, level_enum::critical, params, 2);
    return 0;
}

static cell_t GetName(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    std::size_t bytes = 0;
    if (auto err = ctx->StringToLocalUTF8(params[2], params[3], logger->Name().c_str(), &bytes))
    {
        ctx->ReportError("Failed to write name to buffer (error %d)", err);
        return 0;
    }
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

static cell_t ShouldLog(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    return logger->ShouldLog(lvl);
}

static cell_t SetPattern(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    char *pattern;
    if (auto err = ctx->LocalToString(params[2], &pattern))
    {
        ctx->ReportError("Invalid pattern (error %d)", err);
        return 0;
    }

    auto type = Log4sp::NumToPatternTimeType(params[3]);

    logger->SetPattern(pattern, type);
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

static cell_t SetFlushLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    logger->SetFlushLevel(lvl);
    return 0;
}

static cell_t ShouldFlush(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    return logger->ShouldFlush(lvl);
}

static cell_t GetThrowLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    return logger->GetThrowLevel();
}

static cell_t SetThrowLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    logger->SetThrowLevel(lvl);
    return 0;
}

static cell_t ShouldThrow(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto lvl = Log4sp::NumToLvl(params[2]);

    return logger->ShouldThrow(lvl);
}

static cell_t GetSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    auto index = static_cast<std::size_t>(params[2]);
    if (index >= logger->Sinks().size())
    {
        ctx->ReportError("Invalid index %u. [0-%u]", index, logger->Sinks().size());
        return BAD_HANDLE;
    }

    // 克隆体所有权归于 native caller
    SourceMod::Handle_t handle = logger->SinksHandle().at(index);
    SourceMod::IdentityToken_t *owner = ctx->GetIdentity();
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto cloned = Log4sp::SinkHandler::Instance().CloneHandle(handle, owner, &security, &error);
    if (!cloned)
    {
        ctx->ReportError("Failed to clone Sink Handle %x (error %d)", handle, error);
        return BAD_HANDLE;
    }
    return cloned;
}

static cell_t GetSinksLength(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);
    return static_cast<cell_t>(logger->Sinks().size());
}

static cell_t AddSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[2], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[2], error);
        return 0;
    }

    SourceMod::IdentityToken_t *owner = myself->GetIdentity();
    security.pOwner = owner;
    auto cloned = Log4sp::SinkHandler::Instance().CloneHandle(params[2], owner, &security, &error);
    if (!cloned)
    {
        ctx->ReportError("Failed to clone Sink Handle %x (error %d)", params[2], error);
        return 0;
    }

    logger->Sinks().push_back(sink);
    logger->SinksHandle().push_back(cloned);

    // HACK: CallbackSink
    if (auto callbackSink = dynamic_cast<Log4sp::Sinks::CallbackSink*>(sink))
        callbackSink->TryRegisterHandle(cloned);
    return 0;
}

static cell_t DropSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[2], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[2], error);
        return 0;
    }

    auto &sinks = logger->Sinks();
    auto &handles = logger->SinksHandle();
    security.pOwner = myself->GetIdentity();

    // Sinks 里可能有多个同样的 Sink
    // 从后向前遍历，避免删除元素时索引变化的问题
    for (int i = static_cast<int>(sinks.size()) - 1; i >= 0; --i)
    {
        if (sinks.at(i) != sink)
            continue;

        // 必须先摘除再释放: FreeHandle 会同步销毁 sink, 其关闭回调 (FileCloseFunction /
        // OnClose) 可能重入插件代码并再次遍历 m_Sinks, 若此时仍持有已释放的对象指针就是 UAF.
        auto cloned = handles.at(i);

        // HACK: CallbackSink
        if (auto callbackSink = dynamic_cast<Log4sp::Sinks::CallbackSink*>(sink))
            callbackSink->DropHandle(cloned);

        sinks.erase(sinks.begin() + i);
        handles.erase(handles.begin() + i);

        error = Log4sp::SinkHandler::Instance().FreeHandle(cloned, &security);
        if (error != SourceMod::HandleError::HandleError_None)
        {
            ctx->ReportError("Failed to free a Sink Handle %x (error %d)", cloned, error);
            return 0;
        }
    }
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

    SourcePawn::IPluginFunction *func = plugin->GetBaseContext()->GetFunctionByIdOrError(params[3]);
    if (!func)
        return 0;

    try
    {
        logger->SetErrorHandler(func);
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
    }
    return 0;
}

static cell_t Equals(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_LOGGER_HANDLE_OR_ERROR(params[1]);

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;
    auto other = Log4sp::LoggerHandler::Instance().ReadHandle(params[2], &security, &error);
    if (!other)
    {
        ctx->ReportError("Invalid Logger Handle %x (error %d)", params[2], error);
        return 0;
    }
    return logger == other;
}

static cell_t IsValid(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    auto logger = Log4sp::LoggerHandler::Instance().ReadHandle(params[1], &security, nullptr);
    return !!logger;
}

const sp_nativeinfo_t LoggerNatives[] =
{
    {"Logger.Logger",                           Logger},
    {"Logger.Log",                              Log},
    {"Logger.LogF",                             LogF},
    {"Logger.LogSrc",                           LogSrc},
    {"Logger.LogSrcF",                          LogSrcF},
    {"Logger.LogLoc",                           LogLoc},
    {"Logger.LogLocF",                          LogLocF},
    {"Logger.LogStackTrace",                    LogStackTrace},
    {"Logger.LogStackTraceF",                   LogStackTraceF},
    {"Logger.Trace",                            Trace},
    {"Logger.TraceF",                           TraceF},
    {"Logger.Debug",                            Debug},
    {"Logger.DebugF",                           DebugF},
    {"Logger.Info",                             Info},
    {"Logger.InfoF",                            InfoF},
    {"Logger.Warn",                             Warn},
    {"Logger.WarnF",                            WarnF},
    {"Logger.Error",                            Error},
    {"Logger.ErrorF",                           ErrorF},
    {"Logger.Fatal",                            Fatal},
    {"Logger.FatalF",                           FatalF},
    {"Logger.GetName",                          GetName},
    {"Logger.GetNameLength",                    GetNameLength},
    {"Logger.GetLevel",                         GetLevel},
    {"Logger.SetLevel",                         SetLevel},
    {"Logger.ShouldLog",                        ShouldLog},
    {"Logger.SetPattern",                       SetPattern},
    {"Logger.Flush",                            Flush},
    {"Logger.GetFlushLevel",                    GetFlushLevel},
    {"Logger.SetFlushLevel",                    SetFlushLevel},
    {"Logger.ShouldFlush",                      ShouldFlush},
    {"Logger.GetThrowLevel",                    GetThrowLevel},
    {"Logger.SetThrowLevel",                    SetThrowLevel},
    {"Logger.ShouldThrow",                      ShouldThrow},
    {"Logger.GetSink",                          GetSink},
    {"Logger.GetSinksLength",                   GetSinksLength},
    {"Logger.AddSink",                          AddSink},
    {"Logger.DropSink",                         DropSink},
    {"Logger.SetErrorHandler",                  SetErrorHandler},
    {"Logger.Equals",                           Equals},
    {"Logger.IsValid",                          IsValid},

    {nullptr,                                   nullptr}
};


#include "spdlog/sinks/stdout_sinks.h"

#include "log4sp/logger.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_handler.h"


static cell_t ServerConsoleSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    std::shared_ptr<spdlog::sinks::stdout_sink_st> sink;
    try
    {
        sink = std::make_shared<spdlog::sinks::stdout_sink_st>();
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());
    SourceMod::HandleError error;

    auto handle = Log4sp::SinkHandler::Instance().CreateHandle(sink, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("Failed to creates a ServerConsoleSink Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t ServerConsoleSink_CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    CTX_LOCAL_TO_STRING(params[1], &name);
    if (Log4sp::LoggerHandler::Instance().FindHandle(name))
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    std::shared_ptr<spdlog::sinks::stdout_sink_st> sink;
    try
    {
        sink = std::make_shared<spdlog::sinks::stdout_sink_st>();
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto logger = std::make_shared<Log4sp::Logger>(name, sink);
    auto handle = Log4sp::LoggerHandler::Instance().CreateHandle(logger, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("Failed to creates a Logger Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

const sp_nativeinfo_t ServerConsoleSinkNatives[] =
{
    {"ServerConsoleSink.ServerConsoleSink",         ServerConsoleSink},

    {"ServerConsoleSink.CreateLogger",              ServerConsoleSink_CreateLogger},

    {nullptr,                                       nullptr}
};

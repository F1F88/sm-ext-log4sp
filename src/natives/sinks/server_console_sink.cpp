#include "spdlog/sinks/stdout_sinks.h"

#include "log4sp/logger.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_handler.h"


static cell_t ServerConsoleSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    spdlog::sinks::stdout_sink_st *sink;
    try
    {
        sink = new spdlog::sinks::stdout_sink_st();
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto handle = Log4sp::SinkHandler::Instance().CreateHandle(sink, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("Failed to creates a ServerConsoleSink Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

const sp_nativeinfo_t ServerConsoleSinkNatives[] =
{
    {"ServerConsoleSink.ServerConsoleSink",         ServerConsoleSink},

    {nullptr,                                       nullptr}
};

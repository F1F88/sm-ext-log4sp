#include "spdlog/sinks/stdout_sinks.h"

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
        delete sink;
        ctx->ReportError("Failed to creates a ServerConsoleSink Handle (error %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t IsValid(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, nullptr);
    return !!sink && !!dynamic_cast<spdlog::sinks::stdout_sink_st*>(sink);
}

const sp_nativeinfo_t ServerConsoleSinkNatives[] =
{
    {"ServerConsoleSink.ServerConsoleSink",         ServerConsoleSink},
    {"ServerConsoleSink.IsValid",                   IsValid},

    {nullptr,                                       nullptr}
};

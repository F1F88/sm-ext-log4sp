#include "log4sp/common.h"
#include "log4sp/adapter/sink_handler.h"
#include "log4sp/sinks/dup_filter_sink.h"


static cell_t DupFilterSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    Log4sp::CellLong *maxSkipDuration;
    if (auto err = ctx->LocalToPhysAddr(params[1], reinterpret_cast<cell_t**>(&maxSkipDuration)))
    {
        ctx->ReportError("Invalid maxSkipDuration (error %d)", err);
        return 0;
    }

    auto msec = std::chrono::milliseconds(maxSkipDuration->ToInt64());

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = new Log4sp::Sinks::DupFilterSink(msec);
    auto handle = Log4sp::SinkHandler::Instance().CreateHandle(sink, &security, nullptr, &error);
    if (!handle)
    {
        delete sink;
        ctx->ReportError("Failed to creates a DupFilterSink Handle (error %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t IsValid(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, nullptr);
    return !!sink && !!dynamic_cast<Log4sp::Sinks::DupFilterSink*>(sink);
}

const sp_nativeinfo_t DupFilterSinkNatives[] =
{
    {"DupFilterSink.DupFilterSink",             DupFilterSink},
    {"DupFilterSink.IsValid",                   IsValid},

    {nullptr,                                   nullptr}
};

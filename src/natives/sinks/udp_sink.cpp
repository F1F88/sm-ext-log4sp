#include "spdlog/details/synchronous_factory.h"
#include "spdlog/sinks/udp_sink.h"

#include "log4sp/logger.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"

using spdlog::sink_ptr;
using spdlog::sinks::udp_sink_config;
using spdlog::sinks::udp_sink_st;

///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                      UDPSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
static cell_t UDPSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *host;
    ctx->LocalToString(params[1], &host);

    uint16_t port = static_cast<uint16_t>(params[2]);

    sink_ptr sink;
    try
    {
        sink = std::make_shared<udp_sink_st>(udp_sink_config(host, port));
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());
    SourceMod::HandleError error;

    auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("SM error! Could not create udp sink handle (error: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t UDPSink_CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name))
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    char *host;
    ctx->LocalToString(params[2], &host);

    uint16_t port = static_cast<uint16_t>(params[3]);

    sink_ptr sink;
    try
    {
        sink = std::make_shared<udp_sink_st>(udp_sink_config(host, port));
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());
    SourceMod::HandleError error;

    auto logger = std::make_shared<log4sp::logger>(name, sink);
    auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("SM error! Could not create udp sink handle (error: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

const sp_nativeinfo_t UDPSinkNatives[] =
{
    {"UDPSink.UDPSink",                             UDPSink},
    {"UDPSink.CreateLogger",                        UDPSink_CreateLogger},

    {nullptr,                                       nullptr}
};

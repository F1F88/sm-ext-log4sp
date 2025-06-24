#include "spdlog/sinks/tcp_sink.h"

#include "log4sp/logger.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"

using spdlog::sink_ptr;
using spdlog::sinks::tcp_sink_config;
using spdlog::sinks::tcp_sink_st;

///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                      TCPSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
static cell_t TCPSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *host;
    CTX_LOCAL_TO_STRING(params[1], &host);

    int port = params[2];
    if (port < 0 || port > UINT16_MAX)
    {
        ctx->ReportError("Invalid port %d. [0 - %d]", port, UINT16_MAX);
        return BAD_HANDLE;
    }

    bool lazyConnect = static_cast<bool>(params[3]);

    tcp_sink_config config(host, port);
    config.lazy_connect = lazyConnect;

    sink_ptr sink;
    try
    {
        sink = std::make_shared<tcp_sink_st>(config);
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
        ctx->ReportError("Failed to creates a TCPSink Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t TCPSink_CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    CTX_LOCAL_TO_STRING(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name))
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    char *host;
    CTX_LOCAL_TO_STRING(params[2], &host);

    int port = params[3];
    if (port < 0 || port > UINT16_MAX)
    {
        ctx->ReportError("Invalid port %d. [0 - %d]", port, UINT16_MAX);
        return BAD_HANDLE;
    }

    bool lazyConnect = static_cast<bool>(params[4]);

    tcp_sink_config config(host, port);
    config.lazy_connect = lazyConnect;

    sink_ptr sink;
    try
    {
        sink = std::make_shared<tcp_sink_st>(config);
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto logger = std::make_shared<log4sp::logger>(name, sink);
    auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("Failed to creates a Logger Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

const sp_nativeinfo_t TCPSinkNatives[] =
{
    {"TCPSink.TCPSink",                             TCPSink},
    {"TCPSink.CreateLogger",                        TCPSink_CreateLogger},

    {nullptr,                                       nullptr}
};

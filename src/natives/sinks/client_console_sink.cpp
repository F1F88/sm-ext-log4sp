#include "log4sp/sinks/client_console_sink.h"

#include "log4sp/adapter/sink_hanlder.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 ClientConsoleSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native ClientConsoleSink(bool multiThread = false);
 */
static cell_t ClientConsoleSink(IPluginContext *ctx, const cell_t *params)
{
    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    bool multiThread = static_cast<bool>(params[1]);
    if (!multiThread)
    {
        auto sink   = std::make_shared<log4sp::sinks::client_console_sink_st>();
        auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
        if (handle == BAD_HANDLE)
        {
            ctx->ReportError("Allocation of ClientConsoleSink handle failed. (err: %d)", handle, error);
            return BAD_HANDLE;
        }

        return handle;
    }
    else
    {
        auto sink   = std::make_shared<log4sp::sinks::client_console_sink_mt>();
        auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
        if (handle == BAD_HANDLE)
        {
            ctx->ReportError("Allocation of multi thread ClientConsoleSink handle failed. (err: %d)", handle, error);
            return BAD_HANDLE;
        }

        return handle;
    }
}

/**
 * public native void SetFilter(SinkClientFilter filter);
 *
 * function Action (int client, const char[] name, LogLevel lvl, const char[] msg);
 */
static cell_t ClientConsoleSink_SetFilter(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    spdlog::sink_ptr sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);
    if (sink == nullptr)
    {
        ctx->ReportError("Invalid sink handle. (hdl: %d, err: %d)", handle, error);
        return 0;
    }

    auto funcID   = static_cast<funcid_t>(params[2]);
    auto function = ctx->GetFunctionById(funcID);
    if (function == nullptr)
    {
        ctx->ReportError("Invalid sink client filter function. (funcID: %d)", funcID);
        return 0;
    }

    {
        auto realSink = std::dynamic_pointer_cast<log4sp::sinks::client_console_sink_st>(sink);
        if (realSink != nullptr)
        {
            if (!realSink->set_player_filter(function))
            {
                ctx->ReportError("SM error! Adding filter to ClientConsoleSink failed.");
            }
            return 0;
        }
    }

    {
        auto realSink = std::dynamic_pointer_cast<log4sp::sinks::client_console_sink_mt>(sink);
        if (realSink != nullptr)
        {
            if (!realSink->set_player_filter(function))
            {
                ctx->ReportError("SM error! Adding filter to multi thread ClientConsoleSink failed.");
            }
            return 0;
        }
    }

    ctx->ReportError("Not a valid ClientConsoleSink handle. (hdl: %d)", handle);
    return 0;
}

const sp_nativeinfo_t ClientConsoleSinkNatives[] =
{
    {"ClientConsoleSink.ClientConsoleSink",         ClientConsoleSink},
    {"ClientConsoleSink.SetFilter",                 ClientConsoleSink_SetFilter},

    {nullptr,                                       nullptr}
};

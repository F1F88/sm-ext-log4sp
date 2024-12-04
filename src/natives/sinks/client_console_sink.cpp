#include "log4sp/sinks/client_console_sink.h"

#include "log4sp/sink_register.h"
#include "log4sp/adapter/single_thread_sink.h"
#include "log4sp/adapter/multi_thread_sink.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 ClientConsoleSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native ClientConsoleSink(bool multiThread = false);
 */
static cell_t ClientConsoleSink(IPluginContext *ctx, const cell_t *params)
{
    std::shared_ptr<log4sp::base_sink> sinkAdapter;

    bool multiThread = static_cast<bool>(params[1]);
    if (!multiThread)
    {
        auto sink   = std::make_shared<log4sp::sinks::client_console_sink_st>();
        sinkAdapter = log4sp::single_thread_sink::create(sink, ctx);
    }
    else
    {
        auto sink   = std::make_shared<log4sp::sinks::client_console_sink_mt>();
        sinkAdapter = log4sp::multi_thread_sink::create(sink, ctx);
    }

    if (sinkAdapter == nullptr)
    {
        return BAD_HANDLE;
    }

    return sinkAdapter->handle();
}

/**
 * public native void SetFilter(ClientConsoleSinkFilter filter);
 */
static cell_t ClientConsoleSink_SetFilter(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    auto sinkAdapterPtr = log4sp::base_sink::read(handle, ctx);
    if (sinkAdapterPtr == nullptr)
    {
        return 0;
    }

    auto funcId   = static_cast<funcid_t>(params[2]);
    auto function = ctx->GetFunctionById(funcId);
    if (function == nullptr)
    {
        ctx->ReportError("Invalid client console sink filter function. (funcId=%d)", funcId);
        return 0;
    }

    if (!sinkAdapterPtr->is_multi_thread())
    {
        auto sink = std::dynamic_pointer_cast<log4sp::sinks::client_console_sink_st>(sinkAdapterPtr->raw());
        if (sink == nullptr)
        {
            ctx->ReportError("Unable to cast sink to single thread client_console_sink.");
            return 0;
        }

        if (!sink->set_player_filter(function))
        {
            ctx->ReportError("SM error! Adding client chat sink filter function failed.");
            return 0;
        }
    }
    else
    {
        auto sink = std::dynamic_pointer_cast<log4sp::sinks::client_console_sink_mt>(sinkAdapterPtr->raw());
        if (sink == nullptr)
        {
            ctx->ReportError("Unable to cast sink to multi thread client_console_sink.");
            return 0;
        }

        if (!sink->set_player_filter(function))
        {
            ctx->ReportError("SM error! Adding client chat sink filter function failed.");
            return 0;
        }
    }

    return 0;
}

const sp_nativeinfo_t ClientConsoleSinkNatives[] =
{
    {"ClientConsoleSink.ClientConsoleSink",         ClientConsoleSink},
    {"ClientConsoleSink.SetFilter",                 ClientConsoleSink_SetFilter},

    {nullptr,                                       nullptr}
};

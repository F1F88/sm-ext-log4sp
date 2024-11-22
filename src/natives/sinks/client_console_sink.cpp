#include <log4sp/client_console_sink.h>
#include <log4sp/sink_handle_manager.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 ClientConsoleSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native ClientConsoleSinkST();
 */
static cell_t ClientConsoleSinkST(IPluginContext *ctx, const cell_t *params)
{
    auto data = log4sp::sink_handle_manager::instance().create_client_console_sink_st(ctx);
    if (data == nullptr)
    {
        return BAD_HANDLE;
    }

    return data->handle();
}

/**
 * public native void SetFilter(ClientConsoleSinkFilter filter);
 */
static cell_t ClientConsoleSinkST_SetFilter(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto sink = log4sp::sink_handle_manager::instance().read_handle(ctx, handle);
    if (sink == nullptr)
    {
        return 0;
    }

    auto sinkData = log4sp::sink_handle_manager::instance().get_data(sink);
    if (sinkData == nullptr)
    {
        ctx->ReportError("Fatal internal error, sink data not found. (hdl=%X)", static_cast<int>(handle));
        return 0;
    }

    auto clientConsoleSink = std::dynamic_pointer_cast<log4sp::sinks::client_console_sink_st>(sinkData->sink_ptr());
    if (clientConsoleSink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to client_console_sink_st.");
        return 0;
    }

    IPluginFunction *filter = ctx->GetFunctionById(params[2]);
    if (filter == NULL )
    {
        ctx->ReportError("Invalid param filter %d.", params[2]);
        return 0;
    }

    if (!clientConsoleSink->set_player_filter(filter))
    {
        ctx->ReportError("Set filter failed.");
        return 0;
    }
    return 0;
}


/**
 * public native ClientConsoleSinkMT();
 */
static cell_t ClientConsoleSinkMT(IPluginContext *ctx, const cell_t *params)
{
    auto data = log4sp::sink_handle_manager::instance().create_client_console_sink_mt(ctx);
    if (data == nullptr)
    {
        return BAD_HANDLE;
    }

    return data->handle();
}

/**
 * public native void SetFilter(ClientConsoleSinkFilter filter);
 */
static cell_t ClientConsoleSinkMT_SetFilter(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto sink = log4sp::sink_handle_manager::instance().read_handle(ctx, handle);
    if (sink == nullptr)
    {
        return 0;
    }

    auto sinkData = log4sp::sink_handle_manager::instance().get_data(sink);
    if (sinkData == nullptr)
    {
        ctx->ReportError("Fatal internal error, sink data not found. (hdl=%X)", static_cast<int>(handle));
        return 0;
    }

    auto clientConsoleSink = std::dynamic_pointer_cast<log4sp::sinks::client_console_sink_mt>(sinkData->sink_ptr());
    if (clientConsoleSink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to client_console_sink_mt.");
        return 0;
    }

    IPluginFunction *filter = ctx->GetFunctionById(params[2]);
    if (filter == NULL )
    {
        ctx->ReportError("Invalid param filter %d.", params[2]);
        return 0;
    }

    if (!clientConsoleSink->set_player_filter(filter))
    {
        ctx->ReportError("Set filter failed.");
        return 0;
    }
    return 0;
}

const sp_nativeinfo_t ClientConsoleSinkNatives[] =
{
    {"ClientConsoleSinkST.ClientConsoleSinkST",     ClientConsoleSinkST},
    {"ClientConsoleSinkST.SetFilter",               ClientConsoleSinkST_SetFilter},
    {"ClientConsoleSinkMT.ClientConsoleSinkMT",     ClientConsoleSinkMT},
    {"ClientConsoleSinkMT.SetFilter",               ClientConsoleSinkMT_SetFilter},

    {NULL,                                          NULL}
};

#include <log4sp/common.h>
#include <log4sp/client_console_sink.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 ClientConsoleSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native ClientConsoleSinkST();
 */
static cell_t ClientConsoleSinkST(IPluginContext *ctx, const cell_t *params)
{
    return log4sp::sinks::CreateHandleOrReportError(
        ctx,
        g_ClientConsoleSinkSTHandleType,
        std::make_shared<log4sp::sinks::client_console_sink_st>());
}

/**
 * public native void SetFilter(ClientConsoleSinkFilter filter);
 */
static cell_t ClientConsoleSinkST_SetFilter(IPluginContext *ctx, const cell_t *params)
{
    spdlog::sink_ptr sink = log4sp::sinks::ReadHandleOrReportError(ctx, params[1]);
    if (sink == nullptr)
    {
        return 0;
    }

    auto clientConsoleSink = std::dynamic_pointer_cast<log4sp::sinks::client_console_sink_st>(sink);
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
    return log4sp::sinks::CreateHandleOrReportError(
        ctx,
        g_ClientConsoleSinkMTHandleType,
        std::make_shared<log4sp::sinks::client_console_sink_mt>());
}

/**
 * public native void SetFilter(ClientConsoleSinkFilter filter);
 */
static cell_t ClientConsoleSinkMT_SetFilter(IPluginContext *ctx, const cell_t *params)
{
    spdlog::sink_ptr sink = log4sp::sinks::ReadHandleOrReportError(ctx, params[1]);
    if (sink == nullptr)
    {
        return 0;
    }

    auto clientConsoleSink = std::dynamic_pointer_cast<log4sp::sinks::client_console_sink_mt>(sink);
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

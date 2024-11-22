#include "spdlog/sinks/stdout_sinks.h"

#include <log4sp/sink_handle_manager.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 ServerConsoleSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native ServerConsoleSinkST();
 */
static cell_t ServerConsoleSinkST(IPluginContext *ctx, const cell_t *params)
{
    auto data = log4sp::sink_handle_manager::instance().create_server_console_sink_st(ctx);
    if (data == nullptr)
    {
        return BAD_HANDLE;
    }

    return data->handle();
}

/**
 * public native ServerConsoleSinkMT();
 */
static cell_t ServerConsoleSinkMT(IPluginContext *ctx, const cell_t *params)
{
    auto data = log4sp::sink_handle_manager::instance().create_server_console_sink_mt(ctx);
    if (data == nullptr)
    {
        return BAD_HANDLE;
    }

    return data->handle();
}

const sp_nativeinfo_t ServerConsoleSinkNatives[] =
{
    {"ServerConsoleSinkST.ServerConsoleSinkST",     ServerConsoleSinkST},
    {"ServerConsoleSinkMT.ServerConsoleSinkMT",     ServerConsoleSinkMT},

    {NULL,                                          NULL}
};

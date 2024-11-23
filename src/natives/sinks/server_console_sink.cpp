#include "spdlog/sinks/stdout_sinks.h"

#include <log4sp/sink_handle_manager.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 ServerConsoleSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native ServerConsoleSink(bool async = false);
 */
static cell_t ServerConsoleSink(IPluginContext *ctx, const cell_t *params)
{
    bool async = static_cast<bool>(params[1]);
    auto data = async ? log4sp::sink_handle_manager::instance().create_server_console_sink_st(ctx) :
                        log4sp::sink_handle_manager::instance().create_server_console_sink_mt(ctx);

    if (data == nullptr)
    {
        return BAD_HANDLE;
    }

    return data->handle();
}

const sp_nativeinfo_t ServerConsoleSinkNatives[] =
{
    {"ServerConsoleSink.ServerConsoleSink",         ServerConsoleSink},

    {NULL,                                          NULL}
};

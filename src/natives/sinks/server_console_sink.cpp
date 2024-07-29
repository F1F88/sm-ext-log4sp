#include "spdlog/sinks/stdout_sinks.h"

#include <log4sp/common.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 ServerConsoleSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native ServerConsoleSinkST();
 */
static cell_t ServerConsoleSinkST(IPluginContext *ctx, const cell_t *params)
{
    return log4sp::sinks::CreateHandleOrReportError(
        ctx,
        g_ServerConsoleSinkSTHandleType,
        std::make_shared<spdlog::sinks::stdout_sink_st>());
}

/**
 * public native ServerConsoleSinkMT();
 */
static cell_t ServerConsoleSinkMT(IPluginContext *ctx, const cell_t *params)
{
    return log4sp::sinks::CreateHandleOrReportError(
        ctx,
        g_ServerConsoleSinkSTHandleType,
        std::make_shared<spdlog::sinks::stdout_sink_mt>());
}

const sp_nativeinfo_t ServerConsoleSinkNatives[] =
{
    {"ServerConsoleSinkST.ServerConsoleSinkST",     ServerConsoleSinkST},
    {"ServerConsoleSinkMT.ServerConsoleSinkMT",     ServerConsoleSinkMT},

    {NULL,                                          NULL}
};

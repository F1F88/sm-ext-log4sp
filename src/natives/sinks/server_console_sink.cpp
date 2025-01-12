#include "spdlog/sinks/stdout_sinks.h"

#include "log4sp/adapter/sink_hanlder.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 ServerConsoleSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native ServerConsoleSink();
 */
static cell_t ServerConsoleSink(IPluginContext *ctx, const cell_t *params)
{
    spdlog::sink_ptr sink;
    try
    {
        sink = std::make_shared<spdlog::sinks::stdout_sink_st>();
    }
    catch(const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    Handle_t handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("SM error! Could not create server console sink handle (error: %d)", error);
        return BAD_HANDLE;
    }

    return handle;
}

const sp_nativeinfo_t ServerConsoleSinkNatives[] =
{
    {"ServerConsoleSink.ServerConsoleSink",         ServerConsoleSink},

    {nullptr,                                       nullptr}
};

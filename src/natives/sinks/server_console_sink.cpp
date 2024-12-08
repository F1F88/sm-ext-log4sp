#include "spdlog/sinks/stdout_sinks.h"

#include "log4sp/adapter/sink_hanlder.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 ServerConsoleSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native ServerConsoleSink(bool multiThread = false);
 */
static cell_t ServerConsoleSink(IPluginContext *ctx, const cell_t *params)
{
    HandleSecurity security{ctx->GetIdentity(), myself->GetIdentity()};
    HandleError error;

    bool multiThread = static_cast<bool>(params[1]);
    if (!multiThread)
    {
        auto sink   = std::make_shared<spdlog::sinks::stdout_sink_st>();
        auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
        if (handle == BAD_HANDLE)
        {
            ctx->ReportError("Allocation of ServerConsoleSink handle failed. (err: %d)", handle, error);
            return BAD_HANDLE;
        }

        return handle;
    }
    else
    {
        auto sink   = std::make_shared<spdlog::sinks::stdout_sink_mt>();
        auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
        if (handle == BAD_HANDLE)
        {
            ctx->ReportError("Allocation of multi thread ServerConsoleSink handle failed. (err: %d)", handle, error);
            return BAD_HANDLE;
        }

        return handle;
    }
}

const sp_nativeinfo_t ServerConsoleSinkNatives[] =
{
    {"ServerConsoleSink.ServerConsoleSink",         ServerConsoleSink},

    {nullptr,                                       nullptr}
};

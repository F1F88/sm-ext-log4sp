#include "spdlog/sinks/stdout_sinks.h"

#include "log4sp/sink_register.h"
#include "log4sp/adapter/single_thread_sink.h"
#include "log4sp/adapter/multi_thread_sink.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 ServerConsoleSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native ServerConsoleSink(bool multiThread = false);
 */
static cell_t ServerConsoleSink(IPluginContext *ctx, const cell_t *params)
{
    std::shared_ptr<log4sp::base_sink> sinkAdapter;

    bool multiThread = static_cast<bool>(params[1]);
    if (!multiThread)
    {
        auto sink    = std::make_shared<spdlog::sinks::stdout_sink_st>();
        sinkAdapter  = log4sp::single_thread_sink::create(sink, ctx);
    }
    else
    {
        auto sink    = std::make_shared<spdlog::sinks::stdout_sink_mt>();
        sinkAdapter  = log4sp::multi_thread_sink::create(sink, ctx);
    }

    if (sinkAdapter == nullptr)
    {
        return BAD_HANDLE;
    }

    return sinkAdapter->handle();
}

const sp_nativeinfo_t ServerConsoleSinkNatives[] =
{
    {"ServerConsoleSink.ServerConsoleSink",         ServerConsoleSink},

    {nullptr,                                       nullptr}
};

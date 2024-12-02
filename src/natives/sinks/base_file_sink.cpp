#include "spdlog/sinks/basic_file_sink.h"

#include "log4sp/sink_register.h"
#include "log4sp/adapter/single_thread_sink.h"
#include "log4sp/adapter/multi_thread_sink.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                   BaseFileSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native BaseFileSink(const char[] file, bool truncate = false, bool multiThread = false);
 */
static cell_t BaseFileSink(IPluginContext *ctx, const cell_t *params)
{
    char *file;
    ctx->LocalToString(params[1], &file);
    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    std::shared_ptr<log4sp::base_sink> sinkAdapter;

    auto truncate    = static_cast<bool>(params[2]);
    bool multiThread = static_cast<bool>(params[3]);
    if (!multiThread)
    {
        auto sink   = std::make_shared<spdlog::sinks::basic_file_sink_st>(path, truncate);
        sinkAdapter = log4sp::single_thread_sink::create(sink, ctx);
    }
    else
    {
        auto sink   = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path, truncate);
        sinkAdapter = log4sp::multi_thread_sink::create(sink, ctx);
    }

    if (sinkAdapter == nullptr)
    {
        return BAD_HANDLE;
    }

    return sinkAdapter->handle();
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t BaseFileSink_GetFilename(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    auto sinkAdapterRaw = log4sp::base_sink::read(handle, ctx);
    if (sinkAdapterRaw == nullptr)
    {
        return 0;
    }

    if (!sinkAdapterRaw->is_multi_thread())
    {
        auto sink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink_st>(sinkAdapterRaw->raw());
        if (sink == nullptr)
        {
            ctx->ReportError("Unable to cast sink to single thread basic_file_sink.");
            return 0;
        }
        ctx->StringToLocal(params[2], params[3], sink->filename().c_str());
    }
    else
    {
        auto sink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink_mt>(sinkAdapterRaw->raw());
        if (sink == nullptr)
        {
            ctx->ReportError("Unable to cast sink to multi thread basic_file_sink.");
            return 0;
        }
        ctx->StringToLocal(params[2], params[3], sink->filename().c_str());
    }

    return 0;
}

const sp_nativeinfo_t BaseFileSinkNatives[] =
{
    {"BaseFileSink.BaseFileSink",               BaseFileSink},
    {"BaseFileSink.GetFilename",                BaseFileSink_GetFilename},

    {nullptr,                                   nullptr}
};

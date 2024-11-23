#include "spdlog/sinks/basic_file_sink.h"

#include <log4sp/sink_handle_manager.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                   BaseFileSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native BaseFileSink(const char[] file, bool truncate = false, bool async = false);
 */
static cell_t BaseFileSink(IPluginContext *ctx, const cell_t *params)
{
    char *file;
    ctx->LocalToString(params[1], &file);
    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto truncate = static_cast<bool>(params[2]);
    bool async = static_cast<bool>(params[3]);
    auto data = async ? log4sp::sink_handle_manager::instance().create_base_file_sink_st(ctx, path, truncate) :
                        log4sp::sink_handle_manager::instance().create_base_file_sink_mt(ctx, path, truncate);

    if (data == nullptr)
    {
        return BAD_HANDLE;
    }

    return data->handle();
}

template <typename Mutex>
static void GetFilename(IPluginContext *ctx, const cell_t *params, log4sp::sink_handle_data *data)
{
    auto sink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink<Mutex>>(data->sink_ptr());
    if (sink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to basic_file_sink_%ct.", data->is_multi_threaded() ? 'm' : 's');
        return;
    }
    ctx->StringToLocal(params[2], params[3], sink->filename().c_str());
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t BaseFileSink_GetFilename(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto sink = log4sp::sink_handle_manager::instance().read_handle(ctx, handle);
    if (sink == nullptr)
    {
        return 0;
    }

    auto data = log4sp::sink_handle_manager::instance().get_data(sink);
    if (data == nullptr)
    {
        ctx->ReportError("Fatal internal error, sink data not found. (hdl=%X)", static_cast<int>(handle));
        return 0;
    }

    if (!data->is_multi_threaded())
    {
        GetFilename<spdlog::details::null_mutex>(ctx, params, data);
        return 0;
    }

    GetFilename<std::mutex>(ctx, params, data);
    return 0;
}

const sp_nativeinfo_t BaseFileSinkNatives[] =
{
    {"BaseFileSink.BaseFileSink",               BaseFileSink},
    {"BaseFileSink.GetFilename",                BaseFileSink_GetFilename},

    {NULL,                                      NULL}
};

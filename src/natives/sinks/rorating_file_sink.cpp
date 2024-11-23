#include "spdlog/sinks/rotating_file_sink.h"

#include <log4sp/sink_handle_manager.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 RotatingFileSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native RotatingFileSink(
 *     const char[] file,
 *     const int maxFileSize,
 *     const int maxFiles,
 *     bool rotateOnOpen = false,
 *     bool async = false
 * );
 */
static cell_t RotatingFileSink(IPluginContext *ctx, const cell_t *params)
{
    char *file;
    ctx->LocalToString(params[1], &file);
    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto maxFileSize = static_cast<size_t>(params[2]);
    if (maxFileSize <= 0)
    {
        ctx->ReportError("maxFileSize arg must be an integer greater than 0.");
        return BAD_HANDLE;
    }

    auto maxFiles = static_cast<size_t>(params[3]);
    if (maxFiles > 200000)
    {
        ctx->ReportError("maxFiles arg cannot exceed 200000.");
        return BAD_HANDLE;
    }

    auto rotateOnOpen = static_cast<bool>(params[4]);
    bool async = static_cast<bool>(params[5]);
    auto data = async ? log4sp::sink_handle_manager::instance().create_rotating_file_sink_st(ctx, path, maxFileSize, maxFiles, rotateOnOpen) :
                        log4sp::sink_handle_manager::instance().create_rotating_file_sink_mt(ctx, path, maxFileSize, maxFiles, rotateOnOpen);

    if (data == nullptr)
    {
        return BAD_HANDLE;
    }

    return data->handle();
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t RotatingFileSink_GetFilename(IPluginContext *ctx, const cell_t *params)
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

template <typename Mutex>
static cell_t GetFilename(IPluginContext *ctx, const cell_t *params, log4sp::sink_handle_data *data)
{
    auto sink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink<Mutex>>(data->sink_ptr());
    if (sink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to rotating_file_sink_%ct.", data->is_multi_threaded() ? 'm' : 's');
        return 0;
    }
    ctx->StringToLocal(params[2], params[3], sink->filename().c_str());
}

/**
 * public native void CalcFilename(const char[] file, int index, char[] buffer, int maxlength);
 */
static cell_t RotatingFileSink_CalcFilename(IPluginContext *ctx, const cell_t *params)
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
        CalcFilename<spdlog::details::null_mutex>(ctx, params, data);
        return 0;
    }

    CalcFilename<std::mutex>(ctx, params, data);
    return 0;
}

template <typename Mutex>
static cell_t CalcFilename(IPluginContext *ctx, const cell_t *params, log4sp::sink_handle_data *data)
{
    auto sink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink<Mutex>>(data->sink_ptr());
    if (sink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to rotating_file_sink_%ct.", data->is_multi_threaded() ? 'm' : 's');
        return 0;
    }

    char *file;
    ctx->LocalToString(params[2], &file);
    auto index = static_cast<size_t>(params[3]);

    ctx->StringToLocal(params[4], params[5], sink->calc_filename(file, index).c_str());
}

const sp_nativeinfo_t RotatingFileSinkNatives[] =
{
    {"RotatingFileSink.RotatingFileSink",       RotatingFileSink},
    {"RotatingFileSink.GetFilename",            RotatingFileSink_GetFilename},
    {"RotatingFileSink.CalcFilename",           RotatingFileSink_CalcFilename},

    {NULL,                                      NULL}
};

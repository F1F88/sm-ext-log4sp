#include "spdlog/sinks/basic_file_sink.h"

#include <log4sp/sink_handle_manager.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                   BaseFileSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native BaseFileSinkST(const char[] file, bool truncate = false);
 */
static cell_t BaseFileSinkST(IPluginContext *ctx, const cell_t *params)
{
    char *file;
    ctx->LocalToString(params[1], &file);
    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto truncate = static_cast<bool>(params[2]);

    auto data = log4sp::sink_handle_manager::instance().create_base_file_sink_st(ctx, path, truncate);
    if (data == nullptr)
    {
        return BAD_HANDLE;
    }

    return data->handle();
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t BaseFileSinkST_GetFilename(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto sink = log4sp::sink_handle_manager::instance().read_handle(ctx, handle);
    if (sink == nullptr)
    {
        return 0;
    }

    auto sinkData = log4sp::sink_handle_manager::instance().get_data(sink);
    if (sinkData == nullptr)
    {
        ctx->ReportError("Fatal internal error, sink data not found. (hdl=%X)", static_cast<int>(handle));
        return 0;
    }

    auto baseFileSink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink_st>(sinkData->sink_ptr());
    if (baseFileSink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to basic_file_sink_st.");
        return 0;
    }

    ctx->StringToLocal(params[2], params[3], baseFileSink->filename().c_str());
    return 0;
}

/**
 * public native BaseFileSinkMT(const char[] file, bool truncate = false);
 */
static cell_t BaseFileSinkMT(IPluginContext *ctx, const cell_t *params)
{
    char *file;
    ctx->LocalToString(params[1], &file);
    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto truncate = static_cast<bool>(params[2]);

    auto data = log4sp::sink_handle_manager::instance().create_base_file_sink_mt(ctx, path, truncate);
    if (data == nullptr)
    {
        return BAD_HANDLE;
    }

    return data->handle();
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t BaseFileSinkMT_GetFilename(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto sink = log4sp::sink_handle_manager::instance().read_handle(ctx, handle);
    if (sink == nullptr)
    {
        return 0;
    }

    auto sinkData = log4sp::sink_handle_manager::instance().get_data(sink);
    if (sinkData == nullptr)
    {
        ctx->ReportError("Fatal internal error, sink data not found. (hdl=%X)", static_cast<int>(handle));
        return 0;
    }

    auto baseFileSink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink_mt>(sinkData->sink_ptr());
    if (baseFileSink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to basic_file_sink_st.");
        return 0;
    }

    ctx->StringToLocal(params[2], params[3], baseFileSink->filename().c_str());
    return 0;
}

const sp_nativeinfo_t BaseFileSinkNatives[] =
{
    {"BaseFileSinkST.BaseFileSinkST",           BaseFileSinkST},
    {"BaseFileSinkST.GetFilename",              BaseFileSinkST_GetFilename},
    {"BaseFileSinkMT.BaseFileSinkMT",           BaseFileSinkMT},
    {"BaseFileSinkMT.GetFilename",              BaseFileSinkMT_GetFilename},

    {NULL,                                      NULL}
};

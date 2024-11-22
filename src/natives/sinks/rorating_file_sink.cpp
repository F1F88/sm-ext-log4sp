#include "spdlog/sinks/rotating_file_sink.h"

#include <log4sp/sink_handle_manager.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 RotatingFileSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native RotatingFileSinkST(
 *     const char[] file,
 *     const int maxFileSize,
 *     const int maxFiles,
 *     bool rotateOnOpen = false
 * );
 */
static cell_t RotatingFileSinkST(IPluginContext *ctx, const cell_t *params)
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

    auto data = log4sp::sink_handle_manager::instance().create_rotating_file_sink_st(ctx, path, maxFileSize, maxFiles, rotateOnOpen);
    if (data == nullptr)
    {
        return BAD_HANDLE;
    }

    return data->handle();
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t RotatingFileSinkST_GetFilename(IPluginContext *ctx, const cell_t *params)
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

    auto rotatingFileSink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_st>(sinkData->sink_ptr());
    if (rotatingFileSink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to rotating_file_sink_st.");
        return 0;
    }

    ctx->StringToLocal(params[2], params[3], rotatingFileSink->filename().c_str());
    return 0;
}

/**
 * public native void CalcFilename(const char[] file, int index, char[] buffer, int maxlength);
 */
static cell_t RotatingFileSinkST_CalcFilename(IPluginContext *ctx, const cell_t *params)
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

    auto rotatingFileSink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_st>(sinkData->sink_ptr());
    if (rotatingFileSink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to rotating_file_sink_st.");
        return 0;
    }
    char *file;
    ctx->LocalToString(params[2], &file);
    int index = params[3];

    ctx->StringToLocal(params[4], params[5], rotatingFileSink->calc_filename(file, index).c_str());
    return 0;
}

/**
 * public native RotatingFileSinkMT(
 *     const char[] file,
 *     const int maxFileSize,
 *     const int maxFiles,
 *     bool rotateOnOpen = false
 * );
 */
static cell_t RotatingFileSinkMT(IPluginContext *ctx, const cell_t *params)
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

    auto data = log4sp::sink_handle_manager::instance().create_rotating_file_sink_mt(ctx, path, maxFileSize, maxFiles, rotateOnOpen);
    if (data == nullptr)
    {
        return BAD_HANDLE;
    }

    return data->handle();
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t RotatingFileSinkMT_GetFilename(IPluginContext *ctx, const cell_t *params)
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

    auto rotatingFileSink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_mt>(sinkData->sink_ptr());
    if (rotatingFileSink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to rotating_file_sink_st.");
        return 0;
    }

    ctx->StringToLocal(params[2], params[3], rotatingFileSink->filename().c_str());
    return 0;
}

/**
 * public native void CalcFilename(const char[] file, int index, char[] buffer, int maxlength);
 */
static cell_t RotatingFileSinkMT_CalcFilename(IPluginContext *ctx, const cell_t *params)
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

    auto rotatingFileSink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_mt>(sinkData->sink_ptr());
    if (rotatingFileSink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to rotating_file_sink_st.");
        return 0;
    }
    char *file;
    ctx->LocalToString(params[2], &file);
    int index = params[3];

    ctx->StringToLocal(params[4], params[5], rotatingFileSink->calc_filename(file, index).c_str());
    return 0;
}

const sp_nativeinfo_t RotatingFileSinkNatives[] =
{
    {"RotatingFileSinkST.RotatingFileSinkST",   RotatingFileSinkST},
    {"RotatingFileSinkST.GetFilename",          RotatingFileSinkST_GetFilename},
    {"RotatingFileSinkST.CalcFilename",         RotatingFileSinkST_CalcFilename},
    {"RotatingFileSinkMT.RotatingFileSinkMT",   RotatingFileSinkMT},
    {"RotatingFileSinkMT.GetFilename",          RotatingFileSinkMT_GetFilename},
    {"RotatingFileSinkMT.CalcFilename",         RotatingFileSinkMT_CalcFilename},

    {NULL,                                      NULL}
};

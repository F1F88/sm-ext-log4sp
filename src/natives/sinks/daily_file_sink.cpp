#include "spdlog/sinks/daily_file_sink.h"

#include <log4sp/sink_handle_manager.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                   DailyFileSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native DailyFileSinkST(
 *     const char[] file,
 *     int rotationHour = 0,
 *     int rotationMinute = 0,
 *     bool truncate = false,
 *     int maxFiles = 0
 * );
 */
static cell_t DailyFileSinkST(IPluginContext *ctx, const cell_t *params)
{
    char *file;
    ctx->LocalToString(params[1], &file);
    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto rotationHour = static_cast<int>(params[2]);
    auto rotationMinute = static_cast<int>(params[3]);
    if (rotationHour < 0 || rotationHour > 23 || rotationMinute < 0 || rotationMinute > 59)
    {
        ctx->ReportError("Invalid rotation time in ctor");
        return BAD_HANDLE;
    }

    auto truncate = static_cast<bool>(params[4]);
    auto maxFiles = static_cast<uint16_t>(params[5]);

    auto data = log4sp::sink_handle_manager::instance().create_daily_file_sink_st(ctx, path, rotationHour, rotationMinute, truncate, maxFiles);
    if (data == nullptr)
    {
        return BAD_HANDLE;
    }

    return data->handle();
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t DailyFileSinkST_GetFilename(IPluginContext *ctx, const cell_t *params)
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

    auto dailySink = std::dynamic_pointer_cast<spdlog::sinks::daily_file_sink_st>(sinkData->sink_ptr());
    if (dailySink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to daily_file_sink_st.");
        return 0;
    }

    ctx->StringToLocal(params[2], params[3], dailySink->filename().c_str());
    return 0;
}

/**
 * public native DailyFileSinkMT(
 *     const char[] file,
 *     int rotationHour = 0,
 *     int rotationMinute = 0,
 *     bool truncate = false,
 *     int maxFiles = 0
 * );
 */
static cell_t DailyFileSinkMT(IPluginContext *ctx, const cell_t *params)
{
    char *file;
    ctx->LocalToString(params[1], &file);
    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto rotationHour = static_cast<int>(params[2]);
    auto rotationMinute = static_cast<int>(params[3]);
    if (rotationHour < 0 || rotationHour > 23 || rotationMinute < 0 || rotationMinute > 59)
    {
        ctx->ReportError("Invalid rotation time in ctor");
        return BAD_HANDLE;
    }

    auto truncate = static_cast<bool>(params[4]);
    auto maxFiles = static_cast<uint16_t>(params[5]);

    auto data = log4sp::sink_handle_manager::instance().create_daily_file_sink_mt(ctx, path, rotationHour, rotationMinute, truncate, maxFiles);
    if (data == nullptr)
    {
        return BAD_HANDLE;
    }

    return data->handle();
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t DailyFileSinkMT_GetFilename(IPluginContext *ctx, const cell_t *params)
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

    auto dailySink = std::dynamic_pointer_cast<spdlog::sinks::daily_file_sink_mt>(sinkData->sink_ptr());
    if (dailySink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to daily_file_sink_mt.");
        return 0;
    }

    ctx->StringToLocal(params[2], params[3], dailySink->filename().c_str());
    return 0;
}

const sp_nativeinfo_t DailyFileSinkNatives[] =
{
    {"DailyFileSinkST.DailyFileSinkST",         DailyFileSinkST},
    {"DailyFileSinkST.GetFilename",             DailyFileSinkST_GetFilename},
    {"DailyFileSinkMT.DailyFileSinkMT",         DailyFileSinkMT},
    {"DailyFileSinkMT.GetFilename",             DailyFileSinkMT_GetFilename},

    {NULL,                                      NULL}
};

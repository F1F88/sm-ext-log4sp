#include "spdlog/sinks/daily_file_sink.h"

#include <log4sp/common.h>


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

    int rotationHour = params[2];
    int rotationMinute = params[3];
    if (rotationHour < 0 || rotationHour > 23 || rotationMinute < 0 || rotationMinute > 59)
    {
        ctx->ReportError("Invalid rotation time in ctor");
        return false;
    }

    bool truncate = params[4];
    uint16_t maxFiles = params[5];

    return log4sp::sinks::CreateHandleOrReportError(
        ctx,
        g_DailyFileSinkSTHandleType,
        std::make_shared<spdlog::sinks::daily_file_sink_st>(path, rotationHour, rotationMinute, truncate, maxFiles));
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t DailyFileSinkST_GetFilename(IPluginContext *ctx, const cell_t *params)
{
    spdlog::sink_ptr sink = log4sp::sinks::ReadHandleOrReportError(ctx, params[1]);
    if (sink == nullptr)
    {
        return false;
    }

    auto dailySink = std::dynamic_pointer_cast<spdlog::sinks::daily_file_sink_st>(sink);
    if (dailySink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to daily_file_sink_st.");
        return false;
    }

    ctx->StringToLocal(params[2], params[3], dailySink->filename().data());
    return true;
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

    int rotationHour = params[2];
    int rotationMinute = params[3];
    if (rotationHour < 0 || rotationHour > 23 || rotationMinute < 0 || rotationMinute > 59)
    {
        ctx->ReportError("Invalid rotation time in ctor");
        return false;
    }

    bool truncate = params[4];
    uint16_t maxFiles = params[5];

    return log4sp::sinks::CreateHandleOrReportError(
        ctx,
        g_DailyFileSinkMTHandleType,
        std::make_shared<spdlog::sinks::daily_file_sink_mt>(path, rotationHour, rotationMinute, truncate, maxFiles));
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t DailyFileSinkMT_GetFilename(IPluginContext *ctx, const cell_t *params)
{
    spdlog::sink_ptr sink = log4sp::sinks::ReadHandleOrReportError(ctx, params[1]);
    if (sink == nullptr)
    {
        return false;
    }

    auto dailySink = std::dynamic_pointer_cast<spdlog::sinks::daily_file_sink_mt>(sink);
    if (dailySink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to daily_file_sink_mt.");
        return false;
    }

    ctx->StringToLocal(params[2], params[3], dailySink->filename().data());
    return true;
}

const sp_nativeinfo_t DailyFileSinkNatives[] =
{
    {"DailyFileSinkST.DailyFileSinkST",         DailyFileSinkST},
    {"DailyFileSinkST.GetFilename",             DailyFileSinkST_GetFilename},
    {"DailyFileSinkMT.DailyFileSinkMT",         DailyFileSinkMT},
    {"DailyFileSinkMT.GetFilename",             DailyFileSinkMT_GetFilename},

    {NULL,                                      NULL}
};

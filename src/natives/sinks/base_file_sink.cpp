#include "spdlog/sinks/basic_file_sink.h"

#include <log4sp/common.h>


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

    bool truncate = params[2];

    return log4sp::sinks::CreateHandleOrReportError(
        ctx,
        g_BaseFileSinkSTHandleType,
        std::make_shared<spdlog::sinks::basic_file_sink_st>(path, truncate));
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t BaseFileSinkST_GetFilename(IPluginContext *ctx, const cell_t *params)
{
    spdlog::sink_ptr genericSink = log4sp::sinks::ReadHandleOrReportError(ctx, params[1]);
    if (genericSink == nullptr)
    {
        return 0;
    }

    auto sink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink_st>(genericSink);
    if (sink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to basic_file_sink_st.");
        return 0;
    }

    ctx->StringToLocal(params[2], params[3], sink->filename().data());
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

    bool truncate = params[2];

    return log4sp::sinks::CreateHandleOrReportError(
        ctx,
        g_BaseFileSinkMTHandleType,
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(path, truncate));
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t BaseFileSinkMT_GetFilename(IPluginContext *ctx, const cell_t *params)
{
    spdlog::sink_ptr genericSink = log4sp::sinks::ReadHandleOrReportError(ctx, params[1]);
    if (genericSink == nullptr)
    {
        return 0;
    }

    auto sink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink_mt>(genericSink);
    if (sink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to basic_file_sink_mt.");
        return 0;
    }

    ctx->StringToLocal(params[2], params[3], sink->filename().data());
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

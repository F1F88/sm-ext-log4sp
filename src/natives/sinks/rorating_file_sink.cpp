#include "spdlog/sinks/rotating_file_sink.h"

#include <log4sp/common.h>


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

    size_t maxFileSize = params[2];
    if (maxFileSize <= 0)
    {
        ctx->ReportError("maxFileSize arg must be an integer greater than 0.");
        return BAD_HANDLE;
    }

    size_t maxFiles = params[3];
    if (maxFiles > 200000)
    {
        ctx->ReportError("maxFiles arg cannot exceed 200000.");
        return BAD_HANDLE;
    }

    bool rotateOnOpen = params[4];

    return log4sp::sinks::CreateHandleOrReportError(
        ctx,
        g_RotatingFileSinkSTHandleType,
        std::make_shared<spdlog::sinks::rotating_file_sink_st>(path, maxFileSize, maxFiles, rotateOnOpen));
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t RotatingFileSinkST_GetFilename(IPluginContext *ctx, const cell_t *params)
{
    spdlog::sink_ptr genericSink = log4sp::sinks::ReadHandleOrReportError(ctx, params[1]);
    if (genericSink == nullptr)
    {
        return 0;
    }

    auto sink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_st>(genericSink);
    if (sink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to rotating_file_sink_st.");
        return 0;
    }

    ctx->StringToLocal(params[2], params[3], sink->filename().data());
    return 0;
}

/**
 * public native void CalcFilename(const char[] file, int index, char[] buffer, int maxlength);
 */
static cell_t RotatingFileSinkST_CalcFilename(IPluginContext *ctx, const cell_t *params)
{
    spdlog::sink_ptr genericSink = log4sp::sinks::ReadHandleOrReportError(ctx, params[1]);
    if (genericSink == nullptr)
    {
        return 0;
    }

    auto sink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_st>(genericSink);
    if (sink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to rotating_file_sink_st.");
        return 0;
    }
    char *file;
    ctx->LocalToString(params[2], &file);
    int index = params[3];

    ctx->StringToLocal(params[4], params[5], sink->calc_filename(file, index).data());
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

    size_t maxFileSize = params[2];
    if (maxFileSize <= 0)
    {
        ctx->ReportError("maxFileSize arg must be an integer greater than 0.");
        return BAD_HANDLE;
    }

    size_t maxFiles = params[3];
    if (maxFiles > 200000)
    {
        ctx->ReportError("maxFiles arg cannot exceed 200000.");
        return BAD_HANDLE;
    }

    bool rotateOnOpen = params[4];

    return log4sp::sinks::CreateHandleOrReportError(
        ctx,
        g_RotatingFileSinkMTHandleType,
        std::make_shared<spdlog::sinks::rotating_file_sink_mt>(path, maxFileSize, maxFiles, rotateOnOpen));
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t RotatingFileSinkMT_GetFilename(IPluginContext *ctx, const cell_t *params)
{
    spdlog::sink_ptr genericSink = log4sp::sinks::ReadHandleOrReportError(ctx, params[1]);
    if (genericSink == nullptr)
    {
        return 0;
    }

    auto sink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_mt>(genericSink);
    if (sink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to rotating_file_sink_mt.");
        return 0;
    }

    ctx->StringToLocal(params[2], params[3], sink->filename().data());
    return 0;
}

/**
 * public native void CalcFilename(const char[] file, int index, char[] buffer, int maxlength);
 */
static cell_t RotatingFileSinkMT_CalcFilename(IPluginContext *ctx, const cell_t *params)
{
    spdlog::sink_ptr genericSink = log4sp::sinks::ReadHandleOrReportError(ctx, params[1]);
    if (genericSink == nullptr)
    {
        return 0;
    }

    auto sink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_mt>(genericSink);
    if (sink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to rotating_file_sink_mt.");
        return 0;
    }

    char *file;
    ctx->LocalToString(params[2], &file);
    int index = params[3];

    ctx->StringToLocal(params[4], params[5], sink->calc_filename(file, index).data());
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

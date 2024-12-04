#include "spdlog/sinks/rotating_file_sink.h"

#include "log4sp/sink_register.h"
#include "log4sp/adapter/single_thread_sink.h"
#include "log4sp/adapter/multi_thread_sink.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 RotatingFileSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native RotatingFileSink(
 *     const char[] file,
 *     const int maxFileSize,
 *     const int maxFiles,
 *     bool rotateOnOpen = false,
 *     bool multiThread = false
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

    std::shared_ptr<log4sp::base_sink> sinkAdapter;

    auto rotateOnOpen = static_cast<bool>(params[4]);
    bool multiThread  = static_cast<bool>(params[5]);
    if (!multiThread)
    {
        auto sink     = std::make_shared<spdlog::sinks::rotating_file_sink_st>(path, maxFileSize, maxFiles, rotateOnOpen);
        sinkAdapter   = log4sp::single_thread_sink::create(sink, ctx);
    }
    else
    {
        auto sink     = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(path, maxFileSize, maxFiles, rotateOnOpen);
        sinkAdapter   = log4sp::multi_thread_sink::create(sink, ctx);
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
static cell_t RotatingFileSink_GetFilename(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    auto sinkAdapterPtr = log4sp::base_sink::read(handle, ctx);
    if (sinkAdapterPtr == nullptr)
    {
        return 0;
    }

    if (!sinkAdapterPtr->is_multi_thread())
    {
        auto sink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_st>(sinkAdapterPtr->raw());
        if (sink == nullptr)
        {
            ctx->ReportError("Unable to cast sink to single threaded rotating_file_sink.");
            return 0;
        }
        ctx->StringToLocal(params[2], params[3], sink->filename().c_str());
    }
    else
    {
        auto sink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_mt>(sinkAdapterPtr->raw());
        if (sink == nullptr)
        {
            ctx->ReportError("Unable to cast sink to multi threaded rotating_file_sink.");
            return 0;
        }
        ctx->StringToLocal(params[2], params[3], sink->filename().c_str());
    }

    return 0;
}

/**
 * public native void CalcFilename(const char[] file, int index, char[] buffer, int maxlength);
 */
static cell_t RotatingFileSink_CalcFilename(IPluginContext *ctx, const cell_t *params)
{
    char *file;
    ctx->LocalToString(params[1], &file);
    auto index = static_cast<size_t>(params[2]);

    auto filename = spdlog::sinks::rotating_file_sink_st::calc_filename(file, index);
    ctx->StringToLocal(params[3], params[4], filename.c_str());
    return 0;
}

const sp_nativeinfo_t RotatingFileSinkNatives[] =
{
    {"RotatingFileSink.RotatingFileSink",       RotatingFileSink},
    {"RotatingFileSink.GetFilename",            RotatingFileSink_GetFilename},
    {"RotatingFileSink.CalcFilename",           RotatingFileSink_CalcFilename},

    {nullptr,                                   nullptr}
};

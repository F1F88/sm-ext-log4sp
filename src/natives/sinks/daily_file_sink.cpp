#include "spdlog/sinks/daily_file_sink.h"

#include "log4sp/adapter/sink_hanlder.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                   DailyFileSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native DailyFileSink(
 *     const char[] file,
 *     int rotationHour = 0,
 *     int rotationMinute = 0,
 *     bool truncate = false,
 *     int maxFiles = 0,
 *     bool multiThread = false
 * );
 */
static cell_t DailyFileSink(IPluginContext *ctx, const cell_t *params)
{
    char *file;
    ctx->LocalToString(params[1], &file);

    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto rotationHour   = static_cast<int>(params[2]);
    auto rotationMinute = static_cast<int>(params[3]);
    auto truncate       = static_cast<bool>(params[4]);
    auto maxFiles       = static_cast<uint16_t>(params[5]);
    auto multiThread    = static_cast<bool>(params[6]);

    spdlog::sink_ptr sink;
    try
    {
        if (!multiThread)
        {
            sink = std::make_shared<spdlog::sinks::daily_file_sink_st>(path, rotationHour, rotationMinute, truncate, maxFiles);
        }
        else
        {
            sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(path, rotationHour, rotationMinute, truncate, maxFiles);
        }
    }
    catch(const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    Handle_t handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("Allocation of sink handle failed. (err: %d)", handle, error);
        return BAD_HANDLE;
    }

    return handle;
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t DailyFileSink_GetFilename(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    spdlog::sink_ptr sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);
    if (sink == nullptr)
    {
        ctx->ReportError("Invalid sink handle. (hdl: %d, err: %d)", handle, error);
        return 0;
    }

    {
        auto realSink = std::dynamic_pointer_cast<spdlog::sinks::daily_file_sink_st>(sink);
        if (realSink != nullptr)
        {
            ctx->StringToLocal(params[2], params[3], realSink->filename().c_str());
            return 0;
        }
    }

    {
        auto realSink = std::dynamic_pointer_cast<spdlog::sinks::daily_file_sink_mt>(sink);
        if (realSink != nullptr)
        {
            ctx->StringToLocal(params[2], params[3], realSink->filename().c_str());
            return 0;
        }
    }

    ctx->ReportError("Not a valid DailyFileSink handle. (hdl: %d)", handle);
    return 0;
}

const sp_nativeinfo_t DailyFileSinkNatives[] =
{
    {"DailyFileSink.DailyFileSink",             DailyFileSink},
    {"DailyFileSink.GetFilename",               DailyFileSink_GetFilename},

    {nullptr,                                   nullptr}
};

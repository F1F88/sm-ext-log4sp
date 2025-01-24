#include "spdlog/sinks/daily_file_sink.h"

#include "log4sp/adapter/logger_handler.h"
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
 *     int maxFiles = 0
 * );
 */
static cell_t DailyFileSink(SourcePawn::IPluginContext *ctx, const cell_t *params)
{
    char *file;
    ctx->LocalToString(params[1], &file);

    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto rotationHour   = static_cast<int>(params[2]);
    auto rotationMinute = static_cast<int>(params[3]);
    auto truncate       = static_cast<bool>(params[4]);
    auto maxFiles       = static_cast<uint16_t>(params[5]);

    spdlog::sink_ptr sink;
    try
    {
        sink = std::make_shared<spdlog::sinks::daily_file_sink_st>(path, rotationHour, rotationMinute, truncate, maxFiles);
    }
    catch(const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("SM error! Could not create daily file sink handle (error: %d)", error);
        return BAD_HANDLE;
    }

    return handle;
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t DailyFileSink_GetFilename(SourcePawn::IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
        return 0;
    }

    auto realSink = std::dynamic_pointer_cast<spdlog::sinks::daily_file_sink_st>(sink);
    if (!realSink)
    {
        ctx->ReportError("Invalid daily file sink handle %x (error: %d)", handle, SourceMod::HandleError::HandleError_Parameter);
        return 0;
    }

    size_t bytes;
    ctx->StringToLocalUTF8(params[2], params[3], realSink->filename().c_str(), &bytes);
    return bytes;
}

/**
 * public static native Logger CreateLogger(const char[] name, const char[] file, int hour = 0, int minute = 0, bool truncate = false, int maxFiles = 0);
 */
static cell_t DailyFileSink_CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name) != BAD_HANDLE)
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    char *file;
    ctx->LocalToString(params[2], &file);

    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto hour     = static_cast<int>(params[3]);
    auto minute   = static_cast<int>(params[4]);
    auto truncate = static_cast<bool>(params[5]);
    auto maxFiles = static_cast<uint16_t>(params[6]);

    spdlog::sink_ptr sink;
    try
    {
        sink = std::make_shared<spdlog::sinks::daily_file_sink_st>(path, hour, minute, truncate, maxFiles);
    }
    catch(const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security{ctx->GetIdentity(), myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = std::make_shared<log4sp::logger>(name, sink);
    auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("SM error! Could not create logger handle (error: %d)", error);
        return BAD_HANDLE;
    }

    return handle;
}

const sp_nativeinfo_t DailyFileSinkNatives[] =
{
    {"DailyFileSink.DailyFileSink",             DailyFileSink},
    {"DailyFileSink.GetFilename",               DailyFileSink_GetFilename},

    {"DailyFileSink.CreateLogger",              DailyFileSink_CreateLogger},

    {nullptr,                                   nullptr}
};

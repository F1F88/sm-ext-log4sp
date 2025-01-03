#include "spdlog/sinks/rotating_file_sink.h"

#include "log4sp/adapter/sink_hanlder.h"


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

    auto maxFileSize  = static_cast<size_t>(params[2]);
    auto maxFiles     = static_cast<size_t>(params[3]);
    auto rotateOnOpen = static_cast<bool>(params[4]);
    bool multiThread  = static_cast<bool>(params[5]);

    spdlog::sink_ptr sink;
    try
    {
        if (!multiThread)
        {
            sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(path, maxFileSize, maxFiles, rotateOnOpen);
        }
        else
        {
            sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(path, maxFileSize, maxFiles, rotateOnOpen);
        }
    }
    catch(const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
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
static cell_t RotatingFileSink_GetFilename(IPluginContext *ctx, const cell_t *params)
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
        auto realSink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_st>(sink);
        if (realSink != nullptr)
        {
            ctx->StringToLocal(params[2], params[3], realSink->filename().c_str());
            return 0;
        }
    }

    {
        auto realSink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_mt>(sink);
        if (realSink != nullptr)
        {
            ctx->StringToLocal(params[2], params[3], realSink->filename().c_str());
            return 0;
        }
    }

    ctx->ReportError("Not a valid RotatingFileSink handle. (hdl: %d)", handle);
    return 0;
}

/**
 * public native void RotateNow();
 */
static cell_t RotatingFileSink_RotateNow(IPluginContext *ctx, const cell_t *params)
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
        auto realSink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_st>(sink);
        if (realSink != nullptr)
        {
            try
            {
                realSink->rotate_now();
            }
            catch(const std::exception &ex)
            {
                ctx->ReportError(ex.what());
            }
            return 0;
        }
    }

    {
        auto realSink = std::dynamic_pointer_cast<spdlog::sinks::rotating_file_sink_mt>(sink);
        if (realSink != nullptr)
        {
            try
            {
                realSink->rotate_now();
            }
            catch(const std::exception &ex)
            {
                ctx->ReportError(ex.what());
            }
            return 0;
        }
    }

    ctx->ReportError("Not a valid RotatingFileSink handle. (hdl: %d)", handle);
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
    {"RotatingFileSink.RotateNow",              RotatingFileSink_RotateNow},
    {"RotatingFileSink.CalcFilename",           RotatingFileSink_CalcFilename},

    {nullptr,                                   nullptr}
};

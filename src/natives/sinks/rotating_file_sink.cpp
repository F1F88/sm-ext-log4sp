#include "spdlog/sinks/rotating_file_sink.h"

#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"

using spdlog::sink_ptr;
using spdlog::sinks::rotating_file_sink_mt;
using spdlog::sinks::rotating_file_sink_st;


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 RotatingFileSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native RotatingFileSink(
 *     const char[] file,
 *     const int maxFileSize,
 *     const int maxFiles,
 *     bool rotateOnOpen = false
 * );
 */
static cell_t RotatingFileSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *file;
    ctx->LocalToString(params[1], &file);
    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto maxFileSize  = static_cast<size_t>(params[2]);
    auto maxFiles     = static_cast<size_t>(params[3]);
    auto rotateOnOpen = static_cast<bool>(params[4]);

    sink_ptr sink;
    try
    {
        sink = std::make_shared<rotating_file_sink_st>(path, maxFileSize, maxFiles, rotateOnOpen);
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("SM error! Could not create rotating file sink handle (error: %d)", error);
        return BAD_HANDLE;
    }

    return handle;
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t RotatingFileSink_GetFilename(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", params[1], error);
        return 0;
    }

    size_t bytes{0};
    if (auto realSink = std::dynamic_pointer_cast<rotating_file_sink_st>(sink))
    {
        ctx->StringToLocalUTF8(params[2], params[3], realSink->filename().c_str(), &bytes);
        return static_cast<cell_t>(bytes);
    }

    if (auto realSink = std::dynamic_pointer_cast<rotating_file_sink_mt>(sink))
    {
        ctx->StringToLocalUTF8(params[2], params[3], realSink->filename().c_str(), &bytes);
        return static_cast<cell_t>(bytes);
    }

    ctx->ReportError("Invalid rotating file sink handle %x (error: %d)", params[1], SourceMod::HandleError::HandleError_Parameter);
    return 0;
}

/**
 * public native void RotateNow();
 */
static cell_t RotatingFileSink_RotateNow(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", params[1], error);
        return 0;
    }

    if (auto realSink = std::dynamic_pointer_cast<rotating_file_sink_st>(sink))
    {
        try
        {
            realSink->rotate_now();
        }
        catch (const std::exception &ex)
        {
            ctx->ReportError(ex.what());
        }
        return 0;
    }

    if (auto realSink = std::dynamic_pointer_cast<rotating_file_sink_mt>(sink))
    {
        try
        {
            realSink->rotate_now();
        }
        catch (const std::exception &ex)
        {
            ctx->ReportError(ex.what());
        }
        return 0;
    }

    ctx->ReportError("Invalid basic file sink handle %x (error: %d)", params[1], SourceMod::HandleError::HandleError_Parameter);
    return 0;
}

/**
 * public native void CalcFilename(const char[] file, int index, char[] buffer, int maxlen);
 */
static cell_t RotatingFileSink_CalcFilename(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *file;
    ctx->LocalToString(params[3], &file);
    auto index = static_cast<size_t>(params[4]);

    auto filename = rotating_file_sink_st::calc_filename(file, index);

    size_t bytes{0};
    ctx->StringToLocalUTF8(params[1], params[2], filename.c_str(), &bytes);
    return static_cast<cell_t>(bytes);
}

/**
 * public static native Logger CreateLogger(const char[] name, const char[] file, int maxFileSize, int maxFiles, bool rotateOnOpen = false);
 */
static cell_t RotatingFileSink_CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
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

    auto maxFileSize  = static_cast<size_t>(params[3]);
    auto maxFiles     = static_cast<size_t>(params[4]);
    auto rotateOnOpen = static_cast<bool>(params[5]);

    sink_ptr sink;
    try
    {
        sink = std::make_shared<rotating_file_sink_st>(path, maxFileSize, maxFiles, rotateOnOpen);
    }
    catch (const std::exception &ex)
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

const sp_nativeinfo_t RotatingFileSinkNatives[] =
{
    {"RotatingFileSink.RotatingFileSink",       RotatingFileSink},
    {"RotatingFileSink.GetFilename",            RotatingFileSink_GetFilename},
    {"RotatingFileSink.RotateNow",              RotatingFileSink_RotateNow},

    {"RotatingFileSink.CalcFilename",           RotatingFileSink_CalcFilename},
    {"RotatingFileSink.CreateLogger",           RotatingFileSink_CreateLogger},

    {nullptr,                                   nullptr}
};

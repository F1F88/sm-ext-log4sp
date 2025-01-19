#include "spdlog/sinks/basic_file_sink.h"

#include "log4sp/logger.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                   BaseFileSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native BaseFileSink(const char[] file, bool truncate = false);
 */
static cell_t BaseFileSink(IPluginContext *ctx, const cell_t *params)
{
    char *file;
    ctx->LocalToString(params[1], &file);

    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto truncate    = static_cast<bool>(params[2]);

    spdlog::sink_ptr sink;
    try
    {
        sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(path, truncate);
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
        ctx->ReportError("SM error! Could not create base file sink handle (error: %d)", error);
        return BAD_HANDLE;
    }

    return handle;
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t BaseFileSink_GetFilename(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    spdlog::sink_ptr sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);
    if (sink == nullptr)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
        return 0;
    }

    auto realSink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink_st>(sink);
    if (realSink == nullptr)
    {
        ctx->ReportError("Invalid base file sink handle %x (error: %d)", handle, HandleError_Parameter);
        return 0;
    }

    size_t bytes;
    ctx->StringToLocalUTF8(params[2], params[3], realSink->filename().c_str(), &bytes);
    return bytes;
}

/**
 * public native void Truncate();
 */
static cell_t BaseFileSink_Truncate(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    spdlog::sink_ptr sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);
    if (sink == nullptr)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
        return 0;
    }

    auto realSink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink_st>(sink);
    if (realSink == nullptr)
    {
        ctx->ReportError("Invalid base file sink handle %x (error: %d)", handle, HandleError_Parameter);
        return 0;
    }

    try
    {
        realSink->truncate();
    }
    catch(const std::exception &ex)
    {
        ctx->ReportError(ex.what());
    }

    return 0;
}

/**
 * public static native Logger CreateLogger(const char[] name, const char[] file, bool truncate = false);
 */
static cell_t BaseFileSink_CreateLogger(IPluginContext *ctx, const cell_t *params)
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

    auto truncate = static_cast<bool>(params[3]);

    spdlog::sink_ptr sink;
    try
    {
        sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(path, truncate);
    }
    catch(const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    HandleSecurity security{ctx->GetIdentity(), myself->GetIdentity()};
    HandleError error;

    auto logger = std::make_shared<log4sp::logger>(name, sink);
    auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("SM error! Could not create logger handle (error: %d)", error);
        return BAD_HANDLE;
    }

    return handle;
}

const sp_nativeinfo_t BaseFileSinkNatives[] =
{
    {"BaseFileSink.BaseFileSink",               BaseFileSink},
    {"BaseFileSink.GetFilename",                BaseFileSink_GetFilename},
    {"BaseFileSink.Truncate",                   BaseFileSink_Truncate},

    {"BaseFileSink.CreateLogger",               BaseFileSink_CreateLogger},

    {nullptr,                                   nullptr}
};

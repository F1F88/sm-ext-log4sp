#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"
#include "log4sp/sinks/basic_file_sink.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                  BasicFileSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native BasicFileSink(const char[] file, bool truncate = false);
 */
static cell_t BasicFileSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *file;
    ctx->LocalToString(params[1], &file);

    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto truncate = static_cast<bool>(params[2]);

    log4sp::sink_ptr sink;
    try
    {
        sink = std::make_shared<log4sp::sinks::basic_file_sink>(path, truncate);
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
        ctx->ReportError("SM error! Could not create base file sink handle (error: %d)", error);
        return BAD_HANDLE;
    }

    return handle;
}

/**
 * public native void GetFilename(char[] buffer, int maxlen);
 */
static cell_t BasicFileSink_GetFilename(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
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

    auto realSink = std::dynamic_pointer_cast<log4sp::sinks::basic_file_sink>(sink);
    if (!realSink)
    {
        ctx->ReportError("Invalid base file sink handle %x (error: %d)", handle, SourceMod::HandleError::HandleError_Parameter);
        return 0;
    }

    size_t bytes{0};
    ctx->StringToLocalUTF8(params[2], params[3], realSink->filename().c_str(), &bytes);
    return bytes;
}

/**
 * public native void Truncate();
 */
static cell_t BasicFileSink_Truncate(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
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

    auto realSink = std::dynamic_pointer_cast<log4sp::sinks::basic_file_sink>(sink);
    if (!realSink)
    {
        ctx->ReportError("Invalid base file sink handle %x (error: %d)", handle, SourceMod::HandleError::HandleError_Parameter);
        return 0;
    }

    try
    {
        realSink->truncate();
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
    }

    return 0;
}

/**
 * public static native Logger CreateLogger(const char[] name, const char[] file, bool truncate = false);
 */
static cell_t BasicFileSink_CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
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

    log4sp::sink_ptr sink;
    try
    {
        sink = std::make_shared<log4sp::sinks::basic_file_sink>(path, truncate);
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

const sp_nativeinfo_t BasicFileSinkNatives[] =
{
    {"BasicFileSink.BasicFileSink",             BasicFileSink},
    {"BasicFileSink.GetFilename",               BasicFileSink_GetFilename},
    {"BasicFileSink.Truncate",                  BasicFileSink_Truncate},

    {"BasicFileSink.CreateLogger",              BasicFileSink_CreateLogger},

    {nullptr,                                   nullptr}
};

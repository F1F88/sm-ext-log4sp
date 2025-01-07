#include "spdlog/sinks/basic_file_sink.h"

#include "log4sp/adapter/sink_hanlder.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                   BaseFileSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native BaseFileSink(const char[] file, bool truncate = false, bool multiThread = false);
 */
static cell_t BaseFileSink(IPluginContext *ctx, const cell_t *params)
{
    char *file;
    ctx->LocalToString(params[1], &file);

    char path[PLATFORM_MAX_PATH];
    smutils->BuildPath(Path_Game, path, sizeof(path), "%s", file);

    auto truncate    = static_cast<bool>(params[2]);
    auto multiThread = static_cast<bool>(params[3]);

    spdlog::sink_ptr sink;
    try
    {
        if (!multiThread)
        {
            sink = std::make_shared<spdlog::sinks::basic_file_sink_st>(path, truncate);
        }
        else
        {
            sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path, truncate);
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
static cell_t BaseFileSink_GetFilename(IPluginContext *ctx, const cell_t *params)
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
        auto realSink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink_st>(sink);
        if (realSink != nullptr)
        {
            size_t bytes;
            ctx->StringToLocalUTF8(params[2], params[3], realSink->filename().c_str(), &bytes);
            return bytes;
        }
    }

    {
        auto realSink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink_mt>(sink);
        if (realSink != nullptr)
        {
            size_t bytes;
            ctx->StringToLocalUTF8(params[2], params[3], realSink->filename().c_str(), &bytes);
            return bytes;
        }
    }

    ctx->ReportError("Not a valid BaseFileSink handle. (hdl: %d)", handle);
    return 0;
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
        ctx->ReportError("Invalid sink handle. (hdl: %d, err: %d)", handle, error);
        return 0;
    }

    {
        auto realSink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink_st>(sink);
        if (realSink != nullptr)
        {
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
    }

    {
        auto realSink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink_mt>(sink);
        if (realSink != nullptr)
        {
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
    }

    ctx->ReportError("Not a valid BaseFileSink handle. (hdl: %d)", handle);
    return 0;
}

const sp_nativeinfo_t BaseFileSinkNatives[] =
{
    {"BaseFileSink.BaseFileSink",               BaseFileSink},
    {"BaseFileSink.GetFilename",                BaseFileSink_GetFilename},
    {"BaseFileSink.Truncate",                   BaseFileSink_Truncate},

    {nullptr,                                   nullptr}
};

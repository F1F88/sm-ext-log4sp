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

    auto truncate = static_cast<bool>(params[2]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    bool multiThread = static_cast<bool>(params[3]);
    if (!multiThread)
    {
        auto sink   = std::make_shared<spdlog::sinks::basic_file_sink_st>(path, truncate);
        auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
        if (handle == BAD_HANDLE)
        {
            ctx->ReportError("Allocation of sink handle failed. (err: %d)", handle, error);
            return BAD_HANDLE;
        }

        return handle;
    }
    else
    {
        auto sink   = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path, truncate);
        auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
        if (handle == BAD_HANDLE)
        {
            ctx->ReportError("Allocation of multi thread sink handle failed. (err: %d)", handle, error);
            return BAD_HANDLE;
        }

        return handle;
    }
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
            ctx->StringToLocal(params[2], params[3], realSink->filename().c_str());
            return 0;
        }
    }

    {
        auto realSink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink_mt>(sink);
        if (realSink != nullptr)
        {
            ctx->StringToLocal(params[2], params[3], realSink->filename().c_str());
            return 0;
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
            realSink->truncate();
            return 0;
        }
    }

    {
        auto realSink = std::dynamic_pointer_cast<spdlog::sinks::basic_file_sink_mt>(sink);
        if (realSink != nullptr)
        {
            realSink->truncate();
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

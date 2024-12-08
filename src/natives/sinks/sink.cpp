#include "spdlog/sinks/sink.h"

#include "log4sp/utils.h"
#include "log4sp/adapter/sink_hanlder.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                       Sink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native LogLevel GetLevel();
 */
static cell_t GetLevel(IPluginContext *ctx, const cell_t *params)
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

    return sink->level();
}

/**
 * public native void SetLevel(LogLevel lvl);
 */
static cell_t SetLevel(IPluginContext *ctx, const cell_t *params)
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

    auto lvl = log4sp::cell_to_level(params[2]);

    sink->set_level(lvl);
    return 0;
}

/**
 * public native void SetPattern(const char[] pattern);
 */
static cell_t SetPattern(IPluginContext *ctx, const cell_t *params)
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

    char *pattern;
    ctx->LocalToString(params[2], &pattern);

    sink->set_pattern(pattern);
    return 0;
}

/**
 * public native bool ShouldLog(LogLevel lvl);
 */
static cell_t ShouldLog(IPluginContext *ctx, const cell_t *params)
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

    auto lvl = log4sp::cell_to_level(params[2]);

    return sink->should_log(lvl);
}

/**
 * public native void Log(const char[] name, LogLevel lvl, const char[] msg);
 */
static cell_t Log(IPluginContext *ctx, const cell_t *params)
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

    char *name;
    ctx->LocalToString(params[2], &name);

    auto lvl = log4sp::cell_to_level(params[3]);

    char *msg;
    ctx->LocalToString(params[4], &msg);

    sink->log(spdlog::details::log_msg(name, lvl, msg));
    return 0;
}

/**
 * public native void Flush();
 */
static cell_t Flush(IPluginContext *ctx, const cell_t *params)
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

    sink->flush();
    return 0;
}


const sp_nativeinfo_t SinkNatives[] =
{
    {"Sink.GetLevel",                           GetLevel},
    {"Sink.SetLevel",                           SetLevel},
    {"Sink.SetPattern",                         SetPattern},
    {"Sink.ShouldLog",                          ShouldLog},
    {"Sink.Log",                                Log},
    {"Sink.Flush",                              Flush},

    {nullptr,                                   nullptr}
};

#include "spdlog/sinks/sink.h"

#include "log4sp/utils.h"
#include "log4sp/sink_register.h"
#include "log4sp/adapter/base_sink.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                       Sink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native LogLevel GetLevel();
 */
static cell_t GetLevel(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto sink = log4sp::base_sink::read(handle, ctx);
    if (sink == nullptr)
    {
        return 0;
    }

    return sink->raw()->level();
}

/**
 * public native void SetLevel(LogLevel lvl);
 */
static cell_t SetLevel(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto sink = log4sp::base_sink::read(handle, ctx);
    if (sink == nullptr)
    {
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    sink->raw()->set_level(lvl);
    return 0;
}

/**
 * public native void SetPattern(const char[] pattern);
 */
static cell_t SetPattern(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto sink = log4sp::base_sink::read(handle, ctx);
    if (sink == nullptr)
    {
        return 0;
    }

    char *pattern;
    ctx->LocalToString(params[2], &pattern);

    sink->raw()->set_pattern(pattern);
    return 0;
}

/**
 * public native bool ShouldLog(LogLevel lvl);
 */
static cell_t ShouldLog(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto sink = log4sp::base_sink::read(handle, ctx);
    if (sink == nullptr)
    {
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    return sink->raw()->should_log(lvl);
}

/**
 * public native void Log(const char[] name, LogLevel lvl, const char[] msg);
 */
static cell_t Log(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto sink = log4sp::base_sink::read(handle, ctx);
    if (sink == nullptr)
    {
        return 0;
    }

    char *name;
    ctx->LocalToString(params[2], &name);

    auto lvl = log4sp::cell_to_level(params[3]);

    char *msg;
    ctx->LocalToString(params[4], &msg);

    sink->raw()->log(spdlog::details::log_msg(name, lvl, msg));
    return 0;
}

/**
 * public native void Flush();
 */
static cell_t Flush(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto sink = log4sp::base_sink::read(handle, ctx);
    if (sink == nullptr)
    {
        return 0;
    }

    sink->raw()->flush();
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

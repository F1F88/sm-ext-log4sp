#include "spdlog/sinks/sink.h"

#include <log4sp/utils.h>
#include <log4sp/sink_handle_manager.h>


/**
 * 每一个手动创建的 sink 都会交给注册器管理
 * 添加到 logger 时会从注册器中获取智能指针
 * 当用户 delete 时，注册器会删除 sink 的引用
 * 但不代表这个 sink 会立刻被释放
 * 如果没有任何 logger 引用这个 sink
 * 即智能指针的引用数为 0，这个 sink 才会被立马释放
 */
///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                       Sink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native LogLevel GetLevel();
 */
static cell_t GetLevel(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto sink = log4sp::sink_handle_manager::instance().read_handle(ctx, handle);
    if (sink == nullptr)
    {
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
    auto sink = log4sp::sink_handle_manager::instance().read_handle(ctx, handle);
    if (sink == nullptr)
    {
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    sink->set_level(lvl);
    return 0;
}


/**
 * set formatting for the sinks in this logger.
 * each sink will get a separate instance of the formatter object.
 *
 * set formatting for the sinks in this logger.
 * equivalent to
 *     set_formatter(make_unique<pattern_formatter>(pattern, time_type))
 * Note: each sink will get a new instance of a formatter object, replacing the old one.
 *
 * pattern flags (https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags)
 */
/**
 * public native void SetPattern(const char[] pattern);
 */
static cell_t SetPattern(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto sink = log4sp::sink_handle_manager::instance().read_handle(ctx, handle);
    if (sink == nullptr)
    {
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
    auto sink = log4sp::sink_handle_manager::instance().read_handle(ctx, handle);
    if (sink == nullptr)
    {
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
    auto sink = log4sp::sink_handle_manager::instance().read_handle(ctx, handle);
    if (sink == nullptr)
    {
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
 * public native void LogSrc(const char[] name, LogLevel lvl, const char[] msg);
 */
static cell_t LogSrc(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto sink = log4sp::sink_handle_manager::instance().read_handle(ctx, handle);
    if (sink == nullptr)
    {
        return 0;
    }

    char *name;
    ctx->LocalToString(params[2], &name);

    auto lvl = log4sp::cell_to_level(params[3]);

    char *msg;
    ctx->LocalToString(params[4], &msg);

    auto loc = log4sp::get_plugin_source_loc(ctx);
    sink->log(spdlog::details::log_msg(loc, name, lvl, msg));
    return 0;
}

/**
 * public native void LogLoc(const char[] file, int line, const char[] func, const char[] name, LogLevel lvl, const char[] msg);
 */
static cell_t LogLoc(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto sink = log4sp::sink_handle_manager::instance().read_handle(ctx, handle);
    if (sink == nullptr)
    {
        return 0;
    }

    char *file, *func, *msg;
    ctx->LocalToString(params[2], &file);
    int line = params[3];
    ctx->LocalToString(params[4], &func);
    char *name;
    ctx->LocalToString(params[5], &name);

    auto lvl = log4sp::cell_to_level(params[6]);

    ctx->LocalToString(params[7], &msg);

    spdlog::source_loc loc = {file, line, func};
    sink->log(spdlog::details::log_msg(loc, name, lvl, msg));
    return 0;
}

/**
 * public native void Flush();
 */
static cell_t Flush(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto sink = log4sp::sink_handle_manager::instance().read_handle(ctx, handle);
    if (sink == nullptr)
    {
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
    {"Sink.LogSrc",                             LogSrc},
    {"Sink.LogLoc",                             LogLoc},

    {"Sink.Flush",                              Flush},

    {NULL,                                      NULL}
};

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

    auto sink = log4sp::sink_handler::instance().read_handle_raw(handle, &security, &error);
    if (sink == nullptr)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
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

    auto sink = log4sp::sink_handler::instance().read_handle_raw(handle, &security, &error);
    if (sink == nullptr)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
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

    auto sink = log4sp::sink_handler::instance().read_handle_raw(handle, &security, &error);
    if (sink == nullptr)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
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

    auto sink = log4sp::sink_handler::instance().read_handle_raw(handle, &security, &error);
    if (sink == nullptr)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::cell_to_level(params[2]);

    return sink->should_log(lvl);
}

/**
 * public native void Log(const char[] name, LogLevel lvl, const char[] msg, const char[] file = NULL_STRING, int line = 0, const char[] func = NULL_STRING, int seconds[2] = {0, 0}, int nanoseconds[2] = {0, 0});
 */
static cell_t Log(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle_raw(handle, &security, &error);
    if (sink == nullptr)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
        return 0;
    }

    char *name, *msg;
    ctx->LocalToString(params[2], &name);
    ctx->LocalToString(params[4], &msg);

    auto lvl = log4sp::cell_to_level(params[3]);

    char *file, *func;
    ctx->LocalToStringNULL(params[5], &file);
    ctx->LocalToStringNULL(params[7], &func);

    auto line = static_cast<int>(params[6]);

    spdlog::source_loc loc{};
    if (file && line > 0 && func)
    {
        loc = spdlog::source_loc{file, line, func};
    }

    cell_t *seconds, *nanoseconds;
    ctx->LocalToPhysAddr(params[8], &seconds);
    ctx->LocalToPhysAddr(params[9], &nanoseconds);

    std::chrono::system_clock::time_point logTime{spdlog::details::os::now()};
    if (nanoseconds[0] != 0 || nanoseconds[1] != 0)
    {
        logTime = std::chrono::system_clock::time_point{
            std::chrono::duration_cast<std::chrono::system_clock::duration>(
                std::chrono::nanoseconds{
                    (static_cast<int64_t>(static_cast<uint32_t>(nanoseconds[1])) << 32) |
                    static_cast<uint32_t>(nanoseconds[0])
                }
            )
        };
    }
    else if (seconds[0] != 0 || seconds[1] != 0)
    {
        logTime = std::chrono::system_clock::time_point{
            std::chrono::duration_cast<std::chrono::system_clock::duration>(
                std::chrono::seconds{
                    (static_cast<int64_t>(static_cast<uint32_t>(seconds[1])) << 32) |
                    static_cast<uint32_t>(seconds[0])
                }
            )
        };
    }

    try
    {
        sink->log(spdlog::details::log_msg{logTime, loc, name, lvl, msg});
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
    }
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

    auto sink = log4sp::sink_handler::instance().read_handle_raw(handle, &security, &error);
    if (sink == nullptr)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
        return 0;
    }

    try
    {
        sink->flush();
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
    }
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

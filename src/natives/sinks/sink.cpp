#include "log4sp/common.h"
#include "log4sp/adapter/sink_hanlder.h"
#include "log4sp/sinks/base_sink.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                       Sink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native LogLevel GetLevel();
 */
static cell_t GetLevel(SourcePawn::IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle_raw(handle, &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
        return 0;
    }

    return sink->level();
}

/**
 * public native void SetLevel(LogLevel lvl);
 */
static cell_t SetLevel(SourcePawn::IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle_raw(handle, &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::level::from_number(static_cast<uint32_t>(params[2]));

    sink->set_level(lvl);
    return 0;
}

/**
 * public native void SetPattern(const char[] pattern);
 */
static cell_t SetPattern(SourcePawn::IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle_raw(handle, &security, &error);
    if (!sink)
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
static cell_t ShouldLog(SourcePawn::IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle_raw(handle, &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
        return 0;
    }

    auto lvl = log4sp::level::from_number(static_cast<uint32_t>(params[2]));

    return sink->should_log(lvl);
}

/**
 * public native void Log(const char[] name, LogLevel lvl, const char[] msg, const char[] file = NULL_STRING, int line = 0, const char[] func = NULL_STRING, int seconds[2] = {0, 0}, int nanoseconds[2] = {0, 0});
 */
static cell_t Log(SourcePawn::IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle_raw(handle, &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
        return 0;
    }

    char *name, *msg;
    ctx->LocalToString(params[2], &name);
    ctx->LocalToString(params[4], &msg);

    auto lvl = log4sp::level::from_number(static_cast<uint32_t>(params[3]));

    char *file, *func;
    ctx->LocalToStringNULL(params[5], &file);
    ctx->LocalToStringNULL(params[7], &func);

    auto line = static_cast<uint32_t>(params[6]);

    log4sp::source_loc loc{file, line, func};

    cell_t *seconds, *nanoseconds;
    ctx->LocalToPhysAddr(params[8], &seconds);
    ctx->LocalToPhysAddr(params[9], &nanoseconds);

    std::chrono::system_clock::time_point logTime{log4sp::details::os::now()};
    if (nanoseconds[0] != 0 || nanoseconds[1] != 0)
    {
        logTime = std::chrono::system_clock::time_point{
            std::chrono::duration_cast<std::chrono::system_clock::duration>(
                std::chrono::nanoseconds{
                    log4sp::int32_to_int64(static_cast<uint32_t>(nanoseconds[1]),
                                           static_cast<uint32_t>(nanoseconds[0]))})};
    }
    else if (seconds[0] != 0 || seconds[1] != 0)
    {
        logTime = std::chrono::system_clock::time_point{
            std::chrono::duration_cast<std::chrono::system_clock::duration>(
                std::chrono::seconds{
                    log4sp::int32_to_int64(static_cast<uint32_t>(seconds[1]),
                                           static_cast<uint32_t>(seconds[0]))})};
    }

    try
    {
        sink->log(log4sp::details::log_msg{logTime, loc, name, lvl, msg});
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
static cell_t Flush(SourcePawn::IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle_raw(handle, &security, &error);
    if (!sink)
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

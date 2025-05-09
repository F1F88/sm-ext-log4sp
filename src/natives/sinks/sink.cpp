#include "spdlog/sinks/sink.h"

#include "log4sp/common.h"
#include "log4sp/adapter/sink_hanlder.h"

using spdlog::source_loc;
using spdlog::details::os::now;


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                       Sink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
static cell_t GetLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", params[1], error);
        return 0;
    }

    return sink->level();
}

static cell_t SetLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(params[2]);

    sink->set_level(lvl);
    return 0;
}

static cell_t SetPattern(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", params[1], error);
        return 0;
    }

    char *pattern;
    ctx->LocalToString(params[2], &pattern);

    sink->set_pattern(pattern);
    return 0;
}

static cell_t ShouldLog(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", params[1], error);
        return 0;
    }

    auto lvl = log4sp::num_to_lvl(params[2]);

    return sink->should_log(lvl);
}

static cell_t Log(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", params[1], error);
        return 0;
    }

    char *name, *msg;
    ctx->LocalToString(params[2], &name);
    ctx->LocalToString(params[4], &msg);

    auto lvl = log4sp::num_to_lvl(params[3]);

    char *file, *func;
    ctx->LocalToStringNULL(params[5], &file);
    ctx->LocalToStringNULL(params[7], &func);

    auto line = params[6];

    source_loc loc{file, line, func};

    using std::chrono::system_clock;
    system_clock::time_point logTime{now()};
    if (params[8] != -1)
    {
        logTime = system_clock::time_point{
            std::chrono::duration_cast<system_clock::duration>(std::chrono::seconds{params[8]})};
    }

    try
    {
        sink->log({logTime, loc, name, lvl, msg});
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
    }
    return 0;
}

static cell_t ToPattern(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", params[1], error);
        return 0;
    }

    char *name, *msg;
    ctx->LocalToString(params[4], &name);
    ctx->LocalToString(params[6], &msg);

    auto lvl = log4sp::num_to_lvl(params[5]);

    char *file, *func;
    ctx->LocalToStringNULL(params[7], &file);
    ctx->LocalToStringNULL(params[9], &func);

    int line =params[8];

    source_loc loc{file, line, func};

    using std::chrono::system_clock;
    system_clock::time_point logTime{now()};
    if (params[10] != -1)
    {
        logTime = system_clock::time_point{
            std::chrono::duration_cast<system_clock::duration>(std::chrono::seconds{params[10]})};
    }

    std::string formatted;
    try
    {
        formatted = sink->to_pattern({logTime, loc, name, lvl, msg});
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return 0;
    }

    size_t bytes{0};
    ctx->StringToLocalUTF8(params[2], params[3], formatted.c_str(), &bytes);
    return static_cast<cell_t>(bytes);
}

static cell_t Flush(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle_raw(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", params[1], error);
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
    {"Sink.ToPattern",                          ToPattern},
    {"Sink.Flush",                              Flush},

    {nullptr,                                   nullptr}
};

#include "spdlog/sinks/sink.h"
#include "spdlog/pattern_formatter.h"

#include "log4sp/common.h"
#include "log4sp/adapter/sink_handler.h"
#include "log4sp/sinks/callback_sink.h"


static cell_t Log(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return 0;
    }

    // HACK: CallbackSink
    if (auto callbackSink = dynamic_cast<Log4sp::Sinks::CallbackSink*>(sink))
        callbackSink->TryRegisterHandle(params[1]);

    char *logTime;
    if (auto err = ctx->LocalToString(params[2], &logTime))
    {
        ctx->ReportError("Invalid log time (error %d)", err);
        return 0;
    }

    Log4sp::CellSourceLoc *loc;
    if (auto err = ctx->LocalToPhysAddr(params[3], reinterpret_cast<cell_t**>(&loc)))
    {
        ctx->ReportError("Invalid loc (error %d)", err);
        return 0;
    }

    char *name;
    if (auto err = ctx->LocalToString(params[4], &name))
    {
        ctx->ReportError("Invalid name (error %d)", err);
        return 0;
    }

    auto lvl = Log4sp::NumToLvl(params[5]);

    char *msg;
    if (auto err = ctx->LocalToString(params[6], &msg))
    {
        ctx->ReportError("Invalid msg (error %d)", err);
        return 0;
    }

    using std::chrono::duration_cast;
    using std::chrono::system_clock;
    using spdlog::details::os::now;
    system_clock::time_point logTimePoint;
    try
    {
        auto nanoseconds = std::chrono::nanoseconds(std::stoull(logTime));
        logTimePoint = system_clock::time_point(duration_cast<system_clock::duration>(nanoseconds));
    }
    catch (const std::exception &)
    {
        logTimePoint = now();
    }

    try
    {
        using spdlog::details::log_msg;
        sink->log(log_msg(logTimePoint, loc->ToSourceLoc(), name, lvl, msg));
    }
    catch (const std::exception &ex)
    {
        if (auto err = ctx->StringToLocalUTF8(params[7], params[8], ex.what(), nullptr))
            ctx->ReportError("Failed to write error to buffer (error %d)", err);
        return false;
    }
    return true;
}

static cell_t Flush(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return 0;
    }

    // HACK: CallbackSink
    if (auto callbackSink = dynamic_cast<Log4sp::Sinks::CallbackSink*>(sink))
        callbackSink->TryRegisterHandle(params[1]);

    try
    {
        sink->flush();
    }
    catch (const std::exception &ex)
    {
        if (auto err = ctx->StringToLocalUTF8(params[2], params[3], ex.what(), nullptr))
            ctx->ReportError("Failed to write error to buffer (error %d)", err);
        return false;
    }
    return true;
}

static cell_t SetPattern(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return 0;
    }

    char *pattern;
    if (auto err = ctx->LocalToString(params[2], &pattern))
    {
        ctx->ReportError("Invalid pattern (error %d)", err);
        return 0;
    }

    auto type = Log4sp::NumToPatternTimeType(params[3]);

    sink->set_formatter(spdlog::details::make_unique<spdlog::pattern_formatter>(pattern, type));
    return 0;
}

static cell_t GetLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return 0;
    }

    return sink->level();
}

static cell_t SetLevel(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return 0;
    }

    auto lvl = Log4sp::NumToLvl(params[2]);

    sink->set_level(lvl);
    return 0;
}

static cell_t ShouldLog(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return 0;
    }

    auto lvl = Log4sp::NumToLvl(params[2]);

    return sink->should_log(lvl);
}

static cell_t Equals(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink1 = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink1)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return 0;
    }

    auto sink2 = Log4sp::SinkHandler::Instance().ReadHandle(params[2], &security, nullptr);
    if (!sink2)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[2], error);
        return 0;
    }
    return sink1 == sink2;
}

static cell_t IsValid(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    return !!Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, nullptr);
}


const sp_nativeinfo_t SinkNatives[] =
{
    {"Sink.Log",                                Log},
    {"Sink.Flush",                              Flush},
    {"Sink.SetPattern",                         SetPattern},
    {"Sink.GetLevel",                           GetLevel},
    {"Sink.SetLevel",                           SetLevel},
    {"Sink.ShouldLog",                          ShouldLog},
    {"Sink.Equals",                             Equals},
    {"Sink.IsValid",                            IsValid},

    {nullptr,                                   nullptr}
};

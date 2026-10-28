#include <charconv>

#include "log4sp/common.h"
#include "log4sp/adapter/sink_handler.h"
#include "log4sp/sinks/ringbuffer_sink.h"


static cell_t RingBufferSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto maxSize = static_cast<std::size_t>(params[1]);

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink   = new Log4sp::Sinks::RingBufferSink(maxSize);
    auto handle = Log4sp::SinkHandler::Instance().CreateHandle(sink, &security, nullptr, &error);
    if (!handle)
    {
        delete sink;
        ctx->ReportError("Failed to creates a RingBufferSink Handle (error %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t DrainLatest(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return false;
    }

    auto ringBufferSink = dynamic_cast<Log4sp::Sinks::RingBufferSink*>(sink);
    if (!ringBufferSink)
    {
        ctx->ReportError("Invalid RingBufferSink Handle %x.", params[1]);
        return false;
    }

    SourceMod::IPlugin *plugin;
    if (!params[2])
    {
        plugin = Log4sp::PluginSysFindPluginByCtx(ctx);
    }
    else
    {
        SourceMod::HandleError err;
        plugin = plsys->PluginFromHandle(params[2], &err);
        if (!plugin)
        {
            ctx->ReportError("Invalid Plugin Handle %x (error %d)", params[2], err);
            return false;
        }
    }

    SourcePawn::IPluginFunction *func = plugin->GetBaseContext()->GetFunctionByIdOrError(params[3]);
    if (!func)
        return false;

    // void (LOG4SP_ES_CONST Long logTime, SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg, any data);
    auto fwd = forwards->CreateForwardEx(nullptr,
                SourceMod::ExecType::ET_Ignore,
                6,
                nullptr,
                SourceMod::ParamType::Param_Array,       // logTime
                SourceMod::ParamType::Param_Array,       // loc
                SourceMod::ParamType::Param_String,      // name
                SourceMod::ParamType::Param_Cell,        // lvl
                SourceMod::ParamType::Param_String,      // msg
                SourceMod::ParamType::Param_Cell);       // data
    if (!fwd)
    {
        ctx->ReportError("Failed to create forward.");
        return false;
    }

    if (!fwd->AddFunction(func))
    {
        forwards->ReleaseForward(fwd);
        ctx->ReportError("Failed to add function.");
        return false;
    }

    auto data = params[4];

    bool drained = ringBufferSink->DrainLatest(
        [&ctx, &fwd, &data](const spdlog::details::log_msg_buffer &logMsg)
        {
            auto logTime = Log4sp::CellLong(
                static_cast<std::int64_t>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        logMsg.time.time_since_epoch()).count()));
            if (auto err = fwd->PushArray(reinterpret_cast<cell_t*>(&logTime), sizeof(logTime) / sizeof(cell_t)))
            {
                forwards->ReleaseForward(fwd);
                fwd = nullptr;
                ctx->ReportError("Failed to push logTime into forward (error %d)", err);
                return;
            }

            auto loc = Log4sp::CellSourceLoc(logMsg.source);
            if (auto err = fwd->PushArray(reinterpret_cast<cell_t*>(&loc), sizeof(loc) / sizeof(cell_t)))
            {
                forwards->ReleaseForward(fwd);
                fwd = nullptr;
                ctx->ReportError("Failed to push loc into forward (error %d)", err);
                return;
            }

            std::string name{logMsg.logger_name.data(), logMsg.logger_name.size()};
            if (auto err = fwd->PushString(name.c_str()))
            {
                forwards->ReleaseForward(fwd);
                fwd = nullptr;
                ctx->ReportError("Failed to push name into forward (error %d)", err);
                return;
            }

            if (auto err = fwd->PushCell(logMsg.level))
            {
                forwards->ReleaseForward(fwd);
                fwd = nullptr;
                ctx->ReportError("Failed to push lvl into forward (error %d)", err);
                return;
            }

            std::string msg{logMsg.payload.data(), logMsg.payload.size()};
            if (auto err = fwd->PushString(msg.c_str()))
            {
                forwards->ReleaseForward(fwd);
                fwd = nullptr;
                ctx->ReportError("Failed to push msg into forward (error %d)", err);
                return;
            }

            if (auto err = fwd->PushCell(data))
            {
                forwards->ReleaseForward(fwd);
                fwd = nullptr;
                ctx->ReportError("Failed to push data into forward (error %d)", err);
                return;
            }

            if (auto err = fwd->Execute())
            {
                forwards->ReleaseForward(fwd);
                fwd = nullptr;
                ctx->ReportError("Failed to execute drain latest forward (error %d)", err);
                return;
            }
        });

    if (fwd)
        forwards->ReleaseForward(fwd);
    return drained;
}

static cell_t DrainOldest(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return false;
    }

    auto ringBufferSink = dynamic_cast<Log4sp::Sinks::RingBufferSink*>(sink);
    if (!ringBufferSink)
    {
        ctx->ReportError("Invalid RingBufferSink Handle %x.", params[1]);
        return false;
    }

    SourceMod::IPlugin *plugin;
    if (!params[2])
    {
        plugin = Log4sp::PluginSysFindPluginByCtx(ctx);
    }
    else
    {
        SourceMod::HandleError err;
        plugin = plsys->PluginFromHandle(params[2], &err);
        if (!plugin)
        {
            ctx->ReportError("Invalid Plugin Handle %x (error %d)", params[2], err);
            return false;
        }
    }

    SourcePawn::IPluginFunction *func = plugin->GetBaseContext()->GetFunctionByIdOrError(params[3]);
    if (!func)
        return false;

    // void (LOG4SP_ES_CONST Long logTime, SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg, any data);
    auto fwd = forwards->CreateForwardEx(nullptr,
                SourceMod::ExecType::ET_Ignore,
                6,
                nullptr,
                SourceMod::ParamType::Param_Array,       // logTime
                SourceMod::ParamType::Param_Array,       // loc
                SourceMod::ParamType::Param_String,      // name
                SourceMod::ParamType::Param_Cell,        // lvl
                SourceMod::ParamType::Param_String,      // msg
                SourceMod::ParamType::Param_Cell);       // data
    if (!fwd)
    {
        ctx->ReportError("Failed to create forward.");
        return false;
    }

    if (!fwd->AddFunction(func))
    {
        forwards->ReleaseForward(fwd);
        ctx->ReportError("Failed to add function.");
        return false;
    }

    auto data = params[4];

    bool drained = ringBufferSink->DrainOldest(
        [&ctx, &fwd, &data](const spdlog::details::log_msg_buffer &logMsg)
        {
            auto logTime = Log4sp::CellLong(
                static_cast<std::int64_t>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        logMsg.time.time_since_epoch()).count()));
            if (auto err = fwd->PushArray(reinterpret_cast<cell_t*>(&logTime), sizeof(logTime) / sizeof(cell_t)))
            {
                forwards->ReleaseForward(fwd);
                fwd = nullptr;
                ctx->ReportError("Failed to push logTime into forward (error %d)", err);
                return;
            }

            auto loc = Log4sp::CellSourceLoc(logMsg.source);
            if (auto err = fwd->PushArray(reinterpret_cast<cell_t*>(&loc), sizeof(loc) / sizeof(cell_t)))
            {
                forwards->ReleaseForward(fwd);
                fwd = nullptr;
                ctx->ReportError("Failed to push loc into forward (error %d)", err);
                return;
            }

            std::string name{logMsg.logger_name.data(), logMsg.logger_name.size()};
            if (auto err = fwd->PushString(name.c_str()))
            {
                forwards->ReleaseForward(fwd);
                fwd = nullptr;
                ctx->ReportError("Failed to push name into forward (error %d)", err);
                return;
            }

            if (auto err = fwd->PushCell(logMsg.level))
            {
                forwards->ReleaseForward(fwd);
                fwd = nullptr;
                ctx->ReportError("Failed to push lvl into forward (error %d)", err);
                return;
            }

            std::string msg{logMsg.payload.data(), logMsg.payload.size()};
            if (auto err = fwd->PushString(msg.c_str()))
            {
                forwards->ReleaseForward(fwd);
                fwd = nullptr;
                ctx->ReportError("Failed to push msg into forward (error %d)", err);
                return;
            }

            if (auto err = fwd->PushCell(data))
            {
                forwards->ReleaseForward(fwd);
                fwd = nullptr;
                ctx->ReportError("Failed to push data into forward (error %d)", err);
                return;
            }

            if (auto err = fwd->Execute())
            {
                forwards->ReleaseForward(fwd);
                fwd = nullptr;
                ctx->ReportError("Failed to execute drain oldest forward (error %d)", err);
                return;
            }
        }
    );

    if (fwd)
        forwards->ReleaseForward(fwd);
    return drained;
}

static cell_t GetMaxSize(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return 0;
    }

    auto ringBufferSink = dynamic_cast<Log4sp::Sinks::RingBufferSink*>(sink);
    if (!ringBufferSink)
    {
        ctx->ReportError("Invalid RingBufferSink Handle %x.", params[1]);
        return 0;
    }

    return static_cast<cell_t>(ringBufferSink->GetMaxSize());
}

static cell_t GetSize(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return 0;
    }

    auto ringBufferSink = dynamic_cast<Log4sp::Sinks::RingBufferSink*>(sink);
    if (!ringBufferSink)
    {
        ctx->ReportError("Invalid RingBufferSink Handle %x.", params[1]);
        return 0;
    }

    return static_cast<cell_t>(ringBufferSink->GetSize());
}

static cell_t IsValid(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, nullptr);
    return !!sink && !!dynamic_cast<Log4sp::Sinks::RingBufferSink*>(sink);
}

const sp_nativeinfo_t RingBufferSinkNatives[] =
{
    {"RingBufferSink.RingBufferSink",               RingBufferSink},
    {"RingBufferSink.DrainLatest",                  DrainLatest},
    {"RingBufferSink.DrainOldest",                  DrainOldest},
    {"RingBufferSink.GetMaxSize",                   GetMaxSize},
    {"RingBufferSink.GetSize",                      GetSize},
    {"RingBufferSink.IsValid",                      IsValid},

    {nullptr,                                       nullptr}
};

#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_handler.h"
#include "log4sp/sinks/ringbuffer_sink.h"


static cell_t RingBufferSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto amount = static_cast<std::size_t>(params[1]);

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = new Log4sp::Sinks::RingBufferSinkST(amount);
    auto handle = Log4sp::SinkHandler::Instance().CreateHandle(sink, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("Failed to creates a RingBufferSink Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t RingBufferSink_Drain(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return 0;
    }

    auto ringBufferSink = dynamic_cast<Log4sp::Sinks::RingBufferSinkST*>(sink);
    if (!ringBufferSink)
    {
        ctx->ReportError("Invalid RingBufferSink Handle %x.", params[1]);
        return 0;
    }

    auto func = ctx->GetFunctionById(params[2]);
    if (!func)
    {
        ctx->ReportError("Invalid function id: 0x%08x", params[2]);
        return 0;
    }

    // void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int logTime, any data)
    FWDS_CREATE_EX(nullptr, ET_Ignore, 8, nullptr,
                   Param_String,                            // name
                   Param_Cell,                              // lvl
                   Param_String,                            // msg
                   Param_String,                            // file
                   Param_Cell,                              // line
                   Param_String,                            // func
                   Param_Cell,                              // logTime
                   Param_Cell);                             // data

    FWD_ADD_FUNCTION(func);

    auto data = params[3];

    ringBufferSink->Drain(
        [&fwd, &data](const spdlog::details::log_msg_buffer &log_msg)
        {
            using spdlog::fmt_lib::to_string;
            using std::chrono::duration_cast;
            auto name = to_string(log_msg.logger_name);
            auto payload = to_string(log_msg.payload);
            auto seconds = duration_cast<std::chrono::seconds>(log_msg.time.time_since_epoch());
            auto logTime = static_cast<cell_t>(seconds.count());// FIXME: Possible Year 2038 Problem
            auto file    = log_msg.source.filename ? log_msg.source.filename : "";
            auto func    = log_msg.source.funcname ? log_msg.source.funcname : "";

            FWD_PUSH_STRING(name.c_str());                  // name
            FWD_PUSH_CELL(log_msg.level);                   // lvl
            FWD_PUSH_STRING(payload.c_str());               // msg
            FWD_PUSH_STRING(file);                          // file
            FWD_PUSH_CELL(log_msg.source.line);             // line
            FWD_PUSH_STRING(func);                          // func
            FWD_PUSH_CELL(logTime);                         // logTime
            FWD_PUSH_CELL(data);                            // data
            FWD_EXECUTE();
        }
    );

    forwards->ReleaseForward(fwd);
    return 0;
}

static cell_t RingBufferSink_DrainFormatted(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = Log4sp::SinkHandler::Instance().ReadHandle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid Sink Handle %x (error %d)", params[1], error);
        return 0;
    }

    auto ringBufferSink = dynamic_cast<Log4sp::Sinks::RingBufferSinkST*>(sink);
    if (!ringBufferSink)
    {
        ctx->ReportError("Invalid RingBufferSink Handle %x.", params[1]);
        return 0;
    }

    auto func = ctx->GetFunctionById(params[2]);
    if (!func)
    {
        ctx->ReportError("Invalid function id: 0x%08x", params[2]);
        return 0;
    }

    // void (const char[] msg, any data)
    FWDS_CREATE_EX(nullptr, ET_Ignore, 2, nullptr, Param_String, Param_Cell);
    FWD_ADD_FUNCTION(func);

    auto data = params[3];

    ringBufferSink->DrainFormatted(
        [&fwd, &data](std::string_view msg)
        {
            FWD_PUSH_STRING(msg.data());
            FWD_PUSH_CELL(data);
            FWD_EXECUTE();
        }
    );

    forwards->ReleaseForward(fwd);
    return 0;
}

const sp_nativeinfo_t RingBufferSinkNatives[] =
{
    {"RingBufferSink.RingBufferSink",               RingBufferSink},
    {"RingBufferSink.Drain",                        RingBufferSink_Drain},
    {"RingBufferSink.DrainFormatted",               RingBufferSink_DrainFormatted},

    {nullptr,                                       nullptr}
};

#include "spdlog/sinks/ringbuffer_sink.h"

#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"

using spdlog::sink_ptr;
using spdlog::details::log_msg_buffer;
using spdlog::fmt_lib::to_string;
using spdlog::sinks::ringbuffer_sink_mt;
using spdlog::sinks::ringbuffer_sink_st;

///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 RingBufferSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native RingBufferSink(int amount);
 */
static cell_t RingBufferSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto amount = static_cast<size_t>(params[1]);
    sink_ptr sink = std::make_shared<ringbuffer_sink_st>(amount);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("SM error! Could not create ring buffer sink handle (error: %d)", error);
        return BAD_HANDLE;
    }

    return handle;
}

/**
 * public native void Drain(DrainCallback callback, any data = 0);
 *
 * function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int logTime, any data);
 */
static cell_t RingBufferSink_Drain(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
        return 0;
    }

    auto funcid   = static_cast<funcid_t>(params[2]);
    auto function = ctx->GetFunctionById(funcid);
    if (!function)
    {
        ctx->ReportError("Invalid function id: 0x%08x", funcid);
        return 0;
    }

    SourceMod::IChangeableForward *forward = forwards->CreateForwardEx(nullptr, ET_Ignore, 8, nullptr,
                                                                       Param_String, Param_Cell, Param_String,
                                                                       Param_String, Param_Cell, Param_String,
                                                                       Param_Cell, Param_Cell);
    if (!forward)
    {
        ctx->ReportError("SM error! Could not create ring buffer sink drain forward.");
        return 0;
    }

    if (!forward->AddFunction(function))
    {
        forwards->ReleaseForward(forward);
        ctx->ReportError("SM error! Could not add ring buffer sink drain function.");
        return 0;
    }

    auto data = params[3];

    if (auto realSink = std::dynamic_pointer_cast<ringbuffer_sink_st>(sink))
    {
        std::vector<log_msg_buffer> messages{realSink->last_raw()};
        for (auto log_msg : messages)
        {
            auto name = to_string(log_msg.logger_name);
            auto payload = to_string(log_msg.payload);
            auto logTime = std::chrono::duration_cast<std::chrono::seconds>(log_msg.time.time_since_epoch());

            forward->PushString(name.c_str());
            forward->PushCell(log_msg.level);
            forward->PushString(payload.c_str());

            forward->PushString(log_msg.source.filename);
            forward->PushCell(log_msg.source.line);
            forward->PushString(log_msg.source.funcname);

            forward->PushCell(static_cast<cell_t>(logTime.count()));
            forward->PushCell(data);
#ifndef DEBUG
            forward->Execute();
#else
            assert(forward->Execute() == SP_ERROR_NONE);
#endif
        }
        forwards->ReleaseForward(forward);
        return 0;
    }

    if (auto realSink = std::dynamic_pointer_cast<ringbuffer_sink_mt>(sink))
    {
        std::vector<log_msg_buffer> messages{realSink->last_raw()};
        for (auto log_msg : messages)
        {
            auto name = to_string(log_msg.logger_name);
            auto payload = to_string(log_msg.payload);
            auto logTime = std::chrono::duration_cast<std::chrono::seconds>(log_msg.time.time_since_epoch());

            forward->PushString(name.c_str());
            forward->PushCell(log_msg.level);
            forward->PushString(payload.c_str());

            forward->PushString(log_msg.source.filename);
            forward->PushCell(log_msg.source.line);
            forward->PushString(log_msg.source.funcname);

            forward->PushCell(static_cast<cell_t>(logTime.count()));
            forward->PushCell(data);
#ifndef DEBUG
            forward->Execute();
#else
            assert(forward->Execute() == SP_ERROR_NONE);
#endif
        }
        forwards->ReleaseForward(forward);
        return 0;
    }

    forwards->ReleaseForward(forward);
    ctx->ReportError("Invalid ring buffer sink handle %x (error: %d)", handle, SourceMod::HandleError::HandleError_Parameter);
    return 0;
}

/**
 * public native void DrainFormatted(DrainFormattedCallback callback, any data = 0);
 *
 * function void (const char[] msg, any data);
 */
static cell_t RingBufferSink_DrainFormatted(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto handle = static_cast<SourceMod::Handle_t>(params[1]);

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
        return 0;
    }

    auto function = ctx->GetFunctionById(static_cast<funcid_t>(params[2]));
    if (!function)
    {
        return 0;
    }

    auto forward = forwards->CreateForwardEx(nullptr, ET_Ignore, 2, nullptr, Param_String, Param_Cell);
    if (!forward)
    {
        ctx->ReportError("SM error! Could not create ring buffer sink drain formatted forward.");
        return 0;
    }

    if (!forward->AddFunction(function))
    {
        forwards->ReleaseForward(forward);
        ctx->ReportError("SM error! Could not add ring buffer sink drain formatted function.");
        return 0;
    }

    auto data = params[3];

    if (auto realSink = std::dynamic_pointer_cast<ringbuffer_sink_st>(sink))
    {
        std::vector<std::string> messages{realSink->last_formatted()};
        for (auto message : messages)
        {
            forward->PushString(message.c_str());
            forward->PushCell(data);
#ifndef DEBUG
            forward->Execute();
#else
            assert(forward->Execute() == SP_ERROR_NONE);
#endif
        }
        forwards->ReleaseForward(forward);
        return 0;
    }

    if (auto realSink = std::dynamic_pointer_cast<ringbuffer_sink_mt>(sink))
    {
        std::vector<std::string> messages{realSink->last_formatted()};
        for (auto message : messages)
        {
            forward->PushString(message.c_str());
            forward->PushCell(data);
#ifndef DEBUG
            forward->Execute();
#else
            assert(forward->Execute() == SP_ERROR_NONE);
#endif
        }
        forwards->ReleaseForward(forward);
        return 0;
    }

    forwards->ReleaseForward(forward);
    ctx->ReportError("Invalid ring buffer sink handle %x (error: %d)", handle, SourceMod::HandleError::HandleError_Parameter);
    return 0;
}

/**
 * public static native Logger CreateLogger(const char[] name, int amount);
 */
static cell_t RingBufferSink_CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name) != BAD_HANDLE)
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    auto amount = static_cast<size_t>(params[2]);

    sink_ptr sink = std::make_shared<ringbuffer_sink_st>(amount);

    SourceMod::HandleSecurity security{ctx->GetIdentity(), myself->GetIdentity()};
    SourceMod::HandleError error;

    auto logger = std::make_shared<log4sp::logger>(name, sink);
    auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("SM error! Could not create logger handle (error: %d)", error);
        return BAD_HANDLE;
    }

    return handle;
}

const sp_nativeinfo_t RingBufferSinkNatives[] =
{
    {"RingBufferSink.RingBufferSink",               RingBufferSink},
    {"RingBufferSink.Drain",                        RingBufferSink_Drain},
    {"RingBufferSink.DrainFormatted",               RingBufferSink_DrainFormatted},
    {"RingBufferSink.CreateLogger",                 RingBufferSink_CreateLogger},

    {nullptr,                                       nullptr}
};

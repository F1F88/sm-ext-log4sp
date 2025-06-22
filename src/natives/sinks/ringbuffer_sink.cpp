#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"
#include "log4sp/sinks/ringbuffer_sink.h"

using spdlog::sink_ptr;
using spdlog::details::log_msg_buffer;
using spdlog::fmt_lib::to_string;
using log4sp::sinks::ringbuffer_sink_mt;
using log4sp::sinks::ringbuffer_sink_st;


/**
 * 封装读取 ringbuffer sink handle 代码
 * 这会创建 4 个变量: security, error, sink, ringBufferSink
 *      读取成功时: 继续执行后续代码
 *      读取失败时: 抛出错误并结束执行, 返回 0 (与 BAD_HANDLE 相同)
 */
#define READ_RING_BUFFER_SINK_HANDLE_OR_ERROR(handle)                                               \
    SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());                             \
    SourceMod::HandleError error;                                                                   \
    auto sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);            \
    if (!sink) {                                                                                    \
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);                      \
        return 0;                                                                                   \
    }                                                                                               \
    auto ringBufferSink = std::dynamic_pointer_cast<ringbuffer_sink_st>(sink);                      \
    if (!ringBufferSink) {                                                                          \
        ctx->ReportError("Invalid ring buffer sink handle %x (error: %d)", handle, SourceMod::HandleError::HandleError_Parameter); \
        return 0;                                                                                   \
    }


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 RingBufferSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
static cell_t RingBufferSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto amount = static_cast<size_t>(params[1]);
    sink_ptr sink = std::make_shared<ringbuffer_sink_st>(amount);

    SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());
    SourceMod::HandleError error;

    auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("SM error! Could not create ring buffer sink handle (error: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t RingBufferSink_Drain(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_RING_BUFFER_SINK_HANDLE_OR_ERROR(params[1]);

    auto function = ctx->GetFunctionById(params[2]);
    if (!function)
    {
        ctx->ReportError("Invalid function id: 0x%08x", params[2]);
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

    ringBufferSink->drain([&forward, &data](const log_msg_buffer &log_msg) {
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
    });

    forwards->ReleaseForward(forward);
    return 0;
}

static cell_t RingBufferSink_DrainFormatted(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_RING_BUFFER_SINK_HANDLE_OR_ERROR(params[1]);

    auto function = ctx->GetFunctionById(params[2]);
    if (!function)
    {
        ctx->ReportError("Invalid function id: 0x%08x", params[2]);
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

    ringBufferSink->drain_formatted([&forward, &data](std::string_view msg) {
        forward->PushString(msg.data());
        forward->PushCell(data);
#ifndef DEBUG
        forward->Execute();
#else
        assert(forward->Execute() == SP_ERROR_NONE);
#endif
    });

    forwards->ReleaseForward(forward);
    return 0;
}

static cell_t RingBufferSink_CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name))
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    auto amount = static_cast<size_t>(params[2]);

    sink_ptr sink = std::make_shared<ringbuffer_sink_st>(amount);

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto logger = std::make_shared<log4sp::logger>(name, sink);
    auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error);
    if (!handle)
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

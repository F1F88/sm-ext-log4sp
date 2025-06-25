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
    std::shared_ptr<ringbuffer_sink_st> ringBufferSink;                                             \
    do {                                                                                            \
        SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());                         \
        SourceMod::HandleError error;                                                               \
        auto sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);        \
        if (!sink) {                                                                                \
            ctx->ReportError("Invalid Sink Handle %x (error code: %d)", handle, error);             \
            return 0;                                                                               \
        }                                                                                           \
        ringBufferSink = std::dynamic_pointer_cast<ringbuffer_sink_st>(sink);                       \
        if (!ringBufferSink) {                                                                      \
            ctx->ReportError("Invalid RingBufferSink Handle %x.", handle);                          \
            return 0;                                                                               \
        }                                                                                           \
    } while(0);


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
        ctx->ReportError("Failed to creates a RingBufferSink Handle (error code: %d)", error);
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

    FWD_ADD_FUNCTION(function);

    auto data = params[3];

    ringBufferSink->drain([&forward, &data](const log_msg_buffer &log_msg) {
        auto name = to_string(log_msg.logger_name);
        auto payload = to_string(log_msg.payload);
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(log_msg.time.time_since_epoch());
        auto logTime = static_cast<cell_t>(seconds.count());// FIXME: Possible Year 2038 Problem

        FWD_PUSH_STRING(name.c_str());                      // name
        FWD_PUSH_CELL(log_msg.level);                       // lvl
        FWD_PUSH_STRING(payload.c_str());                   // msg
        FWD_PUSH_STRING(log_msg.source.filename);           // file
        FWD_PUSH_CELL(log_msg.source.line);                 // line
        FWD_PUSH_STRING(log_msg.source.funcname);           // func
        FWD_PUSH_CELL(logTime);                             // logTime
        FWD_PUSH_CELL(data);                                // data
        FWD_EXECUTE();
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

    // void (const char[] msg, any data)
    FWDS_CREATE_EX(nullptr, ET_Ignore, 2, nullptr, Param_String, Param_Cell);
    FWD_ADD_FUNCTION(function);

    auto data = params[3];

    ringBufferSink->drain_formatted([&forward, &data](std::string_view msg) {
        FWD_PUSH_STRING(msg.data());
        FWD_PUSH_CELL(data);
        FWD_EXECUTE();
    });

    forwards->ReleaseForward(forward);
    return 0;
}

static cell_t RingBufferSink_CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    CTX_LOCAL_TO_STRING(params[1], &name);
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
        ctx->ReportError("Failed to creates a Logger Handle (error code: %d)", error);
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

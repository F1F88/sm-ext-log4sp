#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_handler.h"
#include "log4sp/sinks/ringbuffer_sink.h"


/**
 * 封装读取 ringbuffer sink handle 代码
 * 这会创建 4 个变量: security, error, sink, ringBufferSink
 *      读取成功时: 继续执行后续代码
 *      读取失败时: 抛出错误并结束执行, 返回 0 (与 BAD_HANDLE 相同)
 */
#define READ_RING_BUFFER_SINK_HANDLE_OR_ERROR(handle)                                               \
    std::shared_ptr<Log4sp::Sinks::RingBufferSinkST> ringBufferSink;                                \
    {                                                                                               \
        SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());                         \
        SourceMod::HandleError error;                                                               \
        auto sink = Log4sp::SinkHandler::Instance().ReadHandle(handle, &security, &error);          \
        if (!sink)                                                                                  \
        {                                                                                           \
            ctx->ReportError("Invalid Sink Handle %x (error code: %d)", handle, error);             \
            return 0;                                                                               \
        }                                                                                           \
        ringBufferSink = std::dynamic_pointer_cast<Log4sp::Sinks::RingBufferSinkST>(sink);          \
        if (!ringBufferSink)                                                                        \
        {                                                                                           \
            ctx->ReportError("Invalid RingBufferSink Handle %x.", handle);                          \
            return 0;                                                                               \
        }                                                                                           \
    } while(0);


static cell_t RingBufferSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    auto amount = static_cast<std::size_t>(params[1]);

    SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink = std::make_shared<Log4sp::Sinks::RingBufferSinkST>(amount);
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
    READ_RING_BUFFER_SINK_HANDLE_OR_ERROR(params[1]);

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
    READ_RING_BUFFER_SINK_HANDLE_OR_ERROR(params[1]);

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

static cell_t RingBufferSink_CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    CTX_LOCAL_TO_STRING(params[1], &name);
    if (Log4sp::LoggerHandler::Instance().FindHandle(name))
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    auto amount = static_cast<std::size_t>(params[2]);
    auto sink = std::make_shared<Log4sp::Sinks::RingBufferSinkST>(amount);

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto logger = std::make_shared<Log4sp::Logger>(name, sink);
    auto handle = Log4sp::LoggerHandler::Instance().CreateHandle(logger, &security, nullptr, &error);
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

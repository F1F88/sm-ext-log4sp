#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_handler.h"
#include "log4sp/sinks/callback_sink.h"


/**
 * 封装读取 callback sink handle 代码
 * 这会创建 1 个变量: callbackSink
 *      读取成功时: 继续执行后续代码
 *      读取失败时: 抛出错误并结束执行, 返回 0 (与 BAD_HANDLE 相同)
 */
#define READ_CALLBACK_SINK_HANDLE_OR_ERROR(handle)                                                  \
    std::shared_ptr<Log4sp::Sinks::CallbackSink> callbackSink;                                      \
    {                                                                                               \
        SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());                         \
        SourceMod::HandleError error;                                                               \
        auto sink = Log4sp::SinkHandler::Instance().ReadHandle(handle, &security, &error);          \
        if (!sink)                                                                                  \
        {                                                                                           \
            ctx->ReportError("Invalid Sink Handle %x (error code: %d)", handle, error);             \
            return 0;                                                                               \
        }                                                                                           \
        callbackSink = std::dynamic_pointer_cast<Log4sp::Sinks::CallbackSink>(sink);                \
        if (!callbackSink)                                                                          \
        {                                                                                           \
            ctx->ReportError("Invalid CallbackSink Handle %x.", handle);                            \
            return 0;                                                                               \
        }                                                                                           \
    }


static cell_t CallbackSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourcePawn::IPluginFunction *logFunc    = ctx->GetFunctionById(params[1]);
    SourcePawn::IPluginFunction *logPostFunc= ctx->GetFunctionById(params[2]);
    SourcePawn::IPluginFunction *flushFunc  = ctx->GetFunctionById(params[3]);

    std::shared_ptr<Log4sp::Sinks::CallbackSink> sink;
    try
    {
        sink = std::make_shared<Log4sp::Sinks::CallbackSink>(logFunc, logPostFunc, flushFunc);
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());
    SourceMod::HandleError error;

    auto handle = Log4sp::SinkHandler::Instance().CreateHandle(sink, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("Failed to creates a CallbackSink Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t CallbackSink_SetLogCallback(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_CALLBACK_SINK_HANDLE_OR_ERROR(params[1]);

    try
    {
        callbackSink->SetLogCallback(ctx->GetFunctionById(params[2]));
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
    }
    return 0;
}

static cell_t CallbackSink_SetLogPostCallback(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_CALLBACK_SINK_HANDLE_OR_ERROR(params[1]);

    try
    {
        callbackSink->SetLogPostCallback(ctx->GetFunctionById(params[2]));
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
    }
    return 0;
}

static cell_t CallbackSink_SetFlushCallback(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_CALLBACK_SINK_HANDLE_OR_ERROR(params[1]);

    try
    {
        callbackSink->SetFlushCallback(ctx->GetFunctionById(params[2]));
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
    }
    return 0;
}

const sp_nativeinfo_t CallbackSinkNatives[] =
{
    {"CallbackSink.CallbackSink",                   CallbackSink},
    {"CallbackSink.SetLogCallback",                 CallbackSink_SetLogCallback},
    {"CallbackSink.SetLogPostCallback",             CallbackSink_SetLogPostCallback},
    {"CallbackSink.SetFlushCallback",               CallbackSink_SetFlushCallback},

    {nullptr,                                       nullptr}
};

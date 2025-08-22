#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"
#include "log4sp/sinks/callback_sink.hpp"

using spdlog::sink_ptr;
using log4sp::sinks::callback_sink;


/**
 * 封装读取 callback sink handle 代码
 * 这会创建 1 个变量: callbackSink
 *      读取成功时: 继续执行后续代码
 *      读取失败时: 抛出错误并结束执行, 返回 0 (与 BAD_HANDLE 相同)
 */
#define READ_CALLBACK_SINK_HANDLE_OR_ERROR(handle)                                                  \
    std::shared_ptr<callback_sink> callbackSink;                                                    \
    do {                                                                                            \
        SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());                         \
        SourceMod::HandleError error;                                                               \
        auto sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);        \
        if (!sink) {                                                                                \
            ctx->ReportError("Invalid Sink Handle %x (error code: %d)", handle, error);             \
            return 0;                                                                               \
        }                                                                                           \
        callbackSink = std::dynamic_pointer_cast<callback_sink>(sink);                              \
        if (!callbackSink) {                                                                        \
            ctx->ReportError("Invalid CallbackSink Handle %x.", handle);                            \
            return 0;                                                                               \
        }                                                                                           \
    } while(0);


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                  CallbackSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
static cell_t CallbackSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourcePawn::IPluginFunction *logFunction    = ctx->GetFunctionById(params[1]);
    SourcePawn::IPluginFunction *logPostFunction= ctx->GetFunctionById(params[2]);
    SourcePawn::IPluginFunction *flushFunction  = ctx->GetFunctionById(params[3]);
    SourcePawn::IPluginFunction *destoryFunction= ctx->GetFunctionById(params[4]);
    cell_t data = params[5];

    SourceMod::IdentityToken_t *identity = ctx->GetIdentity();
    SourceMod::Handle_t plugin = plsys->FindPluginByContext(ctx->GetContext())->GetMyHandle();

    SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink   = std::make_shared<callback_sink>(logFunction, logPostFunction, flushFunction, destoryFunction, plugin, data);
    auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("Failed to creates a CallbackSink Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t CallbackSink_SetData(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_CALLBACK_SINK_HANDLE_OR_ERROR(params[1]);

    callbackSink->set_data(params[2]);
    return 0;
}

static cell_t CallbackSink_GetData(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_CALLBACK_SINK_HANDLE_OR_ERROR(params[1]);

    return callbackSink->get_data();
}

static cell_t CallbackSink_CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    CTX_LOCAL_TO_STRING(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name))
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    SourcePawn::IPluginFunction *logFunction     = ctx->GetFunctionById(params[2]);
    SourcePawn::IPluginFunction *logPostFunction = ctx->GetFunctionById(params[3]);
    SourcePawn::IPluginFunction *flushFunction   = ctx->GetFunctionById(params[4]);
    SourcePawn::IPluginFunction *destoryFunction = ctx->GetFunctionById(params[5]);
    cell_t data = params[6];

    SourceMod::IdentityToken_t *identity = ctx->GetIdentity();
    SourceMod::Handle_t plugin = plsys->FindPluginByContext(ctx->GetContext())->GetMyHandle();

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink   = std::make_shared<callback_sink>(logFunction, logPostFunction, flushFunction, destoryFunction, plugin, data);
    auto logger = std::make_shared<log4sp::logger>(name, sink);
    auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("Failed to creates a Logger Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

const sp_nativeinfo_t CallbackSinkNatives[] =
{
    {"CallbackSink.CallbackSink",                   CallbackSink},
    {"CallbackSink.SetData",                        CallbackSink_SetData},
    {"CallbackSink.GetData",                        CallbackSink_GetData},
    {"CallbackSink.CreateLogger",                   CallbackSink_CreateLogger},

    {nullptr,                                       nullptr}
};

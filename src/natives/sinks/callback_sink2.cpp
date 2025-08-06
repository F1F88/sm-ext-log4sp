#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"
#include "log4sp/sinks/callback_sink2.hpp"


/**
 * 封装读取 callback2 sink handle 代码
 * 这会创建 1 个变量: callbackSink2
 *      读取成功时: 继续执行后续代码
 *      读取失败时: 抛出错误并结束执行, 返回 0 (与 BAD_HANDLE 相同)
 */
#define READ_CALLBACK_SINK2_HANDLE_OR_ERROR(handle)                                                 \
    std::shared_ptr<log4sp::sinks::callback_sink2> callbackSink2;                                   \
    do {                                                                                            \
        SourceMod::HandleSecurity security(nullptr, myself->GetIdentity());                         \
        SourceMod::HandleError error;                                                               \
        auto sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);        \
        if (!sink) {                                                                                \
            ctx->ReportError("Invalid Sink Handle %x (error code: %d)", handle, error);             \
            return 0;                                                                               \
        }                                                                                           \
        callbackSink2 = std::dynamic_pointer_cast<log4sp::sinks::callback_sink2>(sink);             \
        if (!callbackSink2) {                                                                       \
            ctx->ReportError("Invalid CallbackSink2 Handle %x.", handle);                           \
            return 0;                                                                               \
        }                                                                                           \
    } while(0);


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 CallbackSink2 Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
static cell_t CallbackSink2(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using log4sp::sinks::callback_sink2;
    using SourcePawn::IPluginFunction;

    IPluginFunction *logFn      = ctx->GetFunctionById(params[1]);
    IPluginFunction *logPostFn  = ctx->GetFunctionById(params[2]);
    IPluginFunction *flushFn    = ctx->GetFunctionById(params[3]);
    IPluginFunction *destroyFn  = ctx->GetFunctionById(params[4]);
    cell_t data = params[5];

    SourceMod::Handle_t plugin = plsys->FindPluginByContext(ctx->GetContext())->GetMyHandle();

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink   = std::make_shared<callback_sink2>(logFn, logPostFn, flushFn, destroyFn, plugin, data);
    auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("Failed to creates a CallbackSink2 Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

static cell_t CallbackSink2_SetData(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_CALLBACK_SINK2_HANDLE_OR_ERROR(params[1]);

    callbackSink2->set_data(params[2]);
    return 0;
}

static cell_t CallbackSink2_GetData(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    READ_CALLBACK_SINK2_HANDLE_OR_ERROR(params[1]);

    return callbackSink2->get_data();
}

static cell_t CallbackSink2_CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    using log4sp::sinks::callback_sink2;
    using SourcePawn::IPluginFunction;

    char *name;
    CTX_LOCAL_TO_STRING(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name))
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    IPluginFunction *logFn      = ctx->GetFunctionById(params[2]);
    IPluginFunction *logPostFn  = ctx->GetFunctionById(params[3]);
    IPluginFunction *flushFn    = ctx->GetFunctionById(params[4]);
    IPluginFunction *destroyFn  = ctx->GetFunctionById(params[5]);
    cell_t data = params[6];

    SourceMod::Handle_t plugin = plsys->FindPluginByContext(ctx->GetContext())->GetMyHandle();

    SourceMod::HandleSecurity security(ctx->GetIdentity(), myself->GetIdentity());
    SourceMod::HandleError error;

    auto sink   = std::make_shared<callback_sink2>(logFn, logPostFn, flushFn, destroyFn, plugin, data);
    auto logger = std::make_shared<log4sp::logger>(name, sink);
    auto handle = log4sp::logger_handler::instance().create_handle(logger, &security, nullptr, &error);
    if (!handle)
    {
        ctx->ReportError("Failed to creates a Logger Handle (error code: %d)", error);
        return BAD_HANDLE;
    }
    return handle;
}

const sp_nativeinfo_t CallbackSink2Natives[] =
{
    {"CallbackSink2.CallbackSink2",                 CallbackSink2},
    {"CallbackSink2.SetData",                       CallbackSink2_SetData},
    {"CallbackSink2.GetData",                       CallbackSink2_GetData},
    {"CallbackSink2.CreateLogger",                  CallbackSink2_CreateLogger},

    {nullptr,                                       nullptr}
};

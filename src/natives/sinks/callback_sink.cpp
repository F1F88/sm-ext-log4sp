#include "log4sp/sinks/callback_sink.h"

#include "log4sp/adapter/sink_hanlder.h"

#include "log4sp/utils.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                  CallbackSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native CallbackSink(CustomLogCallback logCallback, CustomFlushCallback flushCallback = INVALID_FUNCTION);
 */
static cell_t CallbackSink(IPluginContext *ctx, const cell_t *params)
{
    auto logFunctionId     = static_cast<funcid_t>(params[1]);
    auto logPostFunctionId = static_cast<funcid_t>(params[2]);
    auto flushFunctionId   = static_cast<funcid_t>(params[3]);

    auto logFunction       = ctx->GetFunctionById(logFunctionId);
    auto logPostFunction   = ctx->GetFunctionById(logPostFunctionId);
    auto flushFunction     = ctx->GetFunctionById(flushFunctionId);

    spdlog::sink_ptr sink;
    try
    {
        sink = std::make_shared<log4sp::sinks::callback_sink>(logFunction, logPostFunction, flushFunction);
    }
    catch(const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    Handle_t handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
    if (handle == BAD_HANDLE)
    {
        ctx->ReportError("SM error! Could not create callback sink handle (error: %d)", error);
        return BAD_HANDLE;
    }

    return handle;
}

/**
 * public native void SetLogCallback(CustomLogCallback logCallback);
 *
 * function void(const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int sec[2], int ns[2]);
 */
static cell_t SetLogCallback(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    spdlog::sink_ptr sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);
    if (sink == nullptr)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
        return 0;
    }

    auto logFunctionId = static_cast<funcid_t>(params[2]);
    auto logFunction   = ctx->GetFunctionById(logFunctionId);

    auto realSink = std::dynamic_pointer_cast<log4sp::sinks::callback_sink>(sink);
    if (realSink == nullptr)
    {
        ctx->ReportError("Invalid callback sink handle %x (error: %d)", handle, HandleError_Parameter);
        return 0;
    }

    try
    {
        realSink->set_log_callback(logFunction);
    }
    catch(const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return 0;
    }

    return 0;
}

/**
 * public native void SetLogPostCallback(CustomLogCallback logPostCallback);
 *
 * function void(const char[] msg);
 */
static cell_t SetLogPostCallback(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    spdlog::sink_ptr sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);
    if (sink == nullptr)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
        return 0;
    }

    auto logPostFunctionId = static_cast<funcid_t>(params[2]);
    auto logPostFunction   = ctx->GetFunctionById(logPostFunctionId);

    auto realSink = std::dynamic_pointer_cast<log4sp::sinks::callback_sink>(sink);
    if (realSink == nullptr)
    {
        ctx->ReportError("Invalid callback sink handle %x (error: %d)", handle, HandleError_Parameter);
        return 0;
    }

    try
    {
        realSink->set_log_post_callback(logPostFunction);
    }
    catch(const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return 0;
    }

    return 0;
}

/**
 * public native void SetFlushCallback(CustomFlushCallback flushCallback);
 *
 * function void();
 */
static cell_t SetFlushCallback(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    spdlog::sink_ptr sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);
    if (sink == nullptr)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
        return 0;
    }

    auto realSink = std::dynamic_pointer_cast<log4sp::sinks::callback_sink>(sink);
    if (realSink == nullptr)
    {
        ctx->ReportError("Invalid callback sink handle %x (error: %d)", handle, HandleError_Parameter);
        return 0;
    }

    auto flushFunctionId = static_cast<funcid_t>(params[2]);
    auto flushFunction = ctx->GetFunctionById(flushFunctionId);

    try
    {
        realSink->set_flush_callback(flushFunction);
    }
    catch(const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return 0;
    }

    return 0;
}

/**
 * public native int ToPattern(char[] buffer, int maxlen,
 *      const char[] name, LogLevel lvl, const char[] msg,
 *      const char[] file = NULL_STRING, int line = 0, const char[] func = NULL_STRING,
 *      int seconds[2] = {0, 0}, int nanoseconds[2] = {0, 0});
 */
static cell_t ToPattern(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    spdlog::sink_ptr sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);
    if (sink == nullptr)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", handle, error);
        return 0;
    }

    auto realSink = std::dynamic_pointer_cast<log4sp::sinks::callback_sink>(sink);
    if (realSink == nullptr)
    {
        ctx->ReportError("Invalid callback sink handle %x (error: %d)", handle, HandleError_Parameter);
        return 0;
    }

    char *name, *msg;
    ctx->LocalToString(params[4], &name);
    ctx->LocalToString(params[6], &msg);

    auto lvl = log4sp::cell_to_level(params[5]);

    char *file, *func;
    ctx->LocalToStringNULL(params[7], &file);
    ctx->LocalToStringNULL(params[9], &func);

    auto line = static_cast<int>(params[8]);

    spdlog::source_loc loc{};
    if (file && line > 0 && func)
    {
        loc = {file, line, func};
    }

    cell_t *seconds, *nanoseconds;
    ctx->LocalToPhysAddr(params[10], &seconds);
    ctx->LocalToPhysAddr(params[11], &nanoseconds);

    std::chrono::system_clock::time_point logTime{spdlog::details::os::now()};
    if (nanoseconds[0] != 0 || nanoseconds[1] != 0)
    {
        logTime = std::chrono::system_clock::time_point{
            std::chrono::duration_cast<std::chrono::system_clock::duration>(
                std::chrono::nanoseconds{
                    (static_cast<int64_t>(static_cast<uint32_t>(nanoseconds[1])) << 32) |
                    static_cast<uint32_t>(nanoseconds[0])
                }
            )
        };
    }
    else if (seconds[0] != 0 || seconds[1] != 0)
    {
        logTime = std::chrono::system_clock::time_point{
            std::chrono::duration_cast<std::chrono::system_clock::duration>(
                std::chrono::seconds{
                    (static_cast<int64_t>(static_cast<uint32_t>(seconds[1])) << 32) |
                    static_cast<uint32_t>(seconds[0])
                }
            )
        };
    }

    std::string formatted;
    try
    {
        formatted = realSink->to_pattern({logTime, loc, name, lvl, msg});
    }
    catch(const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return 0;
    }

    size_t bytes{0};
    ctx->StringToLocalUTF8(params[2], params[3], formatted.c_str(), &bytes);
    return bytes;
}

const sp_nativeinfo_t CallbackSinkNatives[] =
{
    {"CallbackSink.CallbackSink",                   CallbackSink},
    {"CallbackSink.SetLogCallback",                 SetLogCallback},
    {"CallbackSink.SetLogPostCallback",             SetLogPostCallback},
    {"CallbackSink.SetFlushCallback",               SetFlushCallback},
    {"CallbackSink.ToPattern",                      ToPattern},

    {nullptr,                                       nullptr}
};

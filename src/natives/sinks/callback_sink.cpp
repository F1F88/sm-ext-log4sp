#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"
#include "log4sp/sinks/callback_sink.h"

using spdlog::sink_ptr;
using log4sp::sinks::callback_sink;


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                  CallbackSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native CallbackSink(CustomLogCallback logCallback, CustomFlushCallback flushCallback = INVALID_FUNCTION);
 */
static cell_t CallbackSink(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourcePawn::IPluginFunction *logFunction{ctx->GetFunctionById(params[1])};
    SourcePawn::IPluginFunction *logPostFunction{ctx->GetFunctionById(params[2])};
    SourcePawn::IPluginFunction *flushFunction{ctx->GetFunctionById(params[3])};

    sink_ptr sink;
    try
    {
        sink = std::make_shared<callback_sink>(logFunction, logPostFunction, flushFunction);
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
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
 * function void(const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int logTime);
 */
static cell_t CallbackSink_SetLogCallback(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", params[1], error);
        return 0;
    }

    if (auto realSink = std::dynamic_pointer_cast<callback_sink>(sink))
    {
        SourcePawn::IPluginFunction *logFunction = ctx->GetFunctionById(params[2]);
        try
        {
            realSink->set_log_callback(logFunction);
        }
        catch (const std::exception &ex)
        {
            ctx->ReportError(ex.what());
        }
        return 0;
    }

    ctx->ReportError("Invalid callback sink handle %x (error: %d)", params[1], SourceMod::HandleError::HandleError_Parameter);
    return 0;
}

/**
 * public native void SetLogPostCallback(CustomLogCallback logPostCallback);
 *
 * function void(const char[] msg);
 */
static cell_t CallbackSink_SetLogPostCallback(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", params[1], error);
        return 0;
    }

    if (auto realSink = std::dynamic_pointer_cast<callback_sink>(sink))
    {
        SourcePawn::IPluginFunction *logPostFunction = ctx->GetFunctionById(params[2]);
        try
        {
            realSink->set_log_post_callback(logPostFunction);
        }
        catch (const std::exception &ex)
        {
            ctx->ReportError(ex.what());
        }
        return 0;
    }

    ctx->ReportError("Invalid callback sink handle %x (error: %d)", params[1], SourceMod::HandleError::HandleError_Parameter);
    return 0;
}

/**
 * public native void SetFlushCallback(CustomFlushCallback flushCallback);
 *
 * function void();
 */
static cell_t CallbackSink_SetFlushCallback(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    SourceMod::HandleSecurity security{nullptr, myself->GetIdentity()};
    SourceMod::HandleError error;

    auto sink = log4sp::sink_handler::instance().read_handle(params[1], &security, &error);
    if (!sink)
    {
        ctx->ReportError("Invalid sink handle %x (error: %d)", params[1], error);
        return 0;
    }

    if (auto realSink = std::dynamic_pointer_cast<callback_sink>(sink))
    {
        SourcePawn::IPluginFunction *flushCallback = ctx->GetFunctionById(params[2]);
        try
        {
            realSink->set_flush_callback(flushCallback);
        }
        catch (const std::exception &ex)
        {
            ctx->ReportError(ex.what());
        }
        return 0;
    }

    ctx->ReportError("Invalid callback sink handle %x (error: %d)", params[1], SourceMod::HandleError::HandleError_Parameter);
    return 0;
}

/**
 * public static native Logger CreateLogger(const char[] name, CustomLogCallback logCallback, CustomFlushCallback flushCallback = INVALID_FUNCTION);
 */
static cell_t CallbackSink_CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params) noexcept
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name) != BAD_HANDLE)
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    SourcePawn::IPluginFunction *logFunction{ctx->GetFunctionById(params[2])};
    SourcePawn::IPluginFunction *logPostFunction{ctx->GetFunctionById(params[3])};
    SourcePawn::IPluginFunction *flushFunction{ctx->GetFunctionById(params[4])};

    sink_ptr sink;
    try
    {
        sink = std::make_shared<callback_sink>(logFunction, logPostFunction, flushFunction);
    }
    catch (const std::exception &ex)
    {
        ctx->ReportError(ex.what());
        return BAD_HANDLE;
    }

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

const sp_nativeinfo_t CallbackSinkNatives[] =
{
    {"CallbackSink.CallbackSink",                   CallbackSink},
    {"CallbackSink.SetLogCallback",                 CallbackSink_SetLogCallback},
    {"CallbackSink.SetLogPostCallback",             CallbackSink_SetLogPostCallback},
    {"CallbackSink.SetFlushCallback",               CallbackSink_SetFlushCallback},
    {"CallbackSink.CreateLogger",                   CallbackSink_CreateLogger},

    {nullptr,                                       nullptr}
};

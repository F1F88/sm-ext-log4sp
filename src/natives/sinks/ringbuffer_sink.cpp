#include "log4sp/common.h"
#include "log4sp/adapter/logger_handler.h"
#include "log4sp/adapter/sink_hanlder.h"
#include "log4sp/sinks/ringbuffer_sink.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 RingBufferSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native RingBufferSink(int amount);
 */
static cell_t RingBufferSink(SourcePawn::IPluginContext *ctx, const cell_t *params)
{
    auto amount = static_cast<size_t>(params[1]);
    log4sp::sink_ptr sink = std::make_shared<log4sp::sinks::ringbuffer_sink>(amount);

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
 * function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int seconds[2], int nanoseconds[2], any data);
 */
static cell_t RingBufferSink_Drain(SourcePawn::IPluginContext *ctx, const cell_t *params)
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

    auto realSink = std::dynamic_pointer_cast<log4sp::sinks::ringbuffer_sink>(sink);
    if (!realSink)
    {
        ctx->ReportError("Invalid ring buffer sink handle %x (error: %d)", handle, SourceMod::HandleError::HandleError_Parameter);
        return 0;
    }

    auto funcID   = static_cast<funcid_t>(params[2]);
    auto function = ctx->GetFunctionById(funcID);
    if (!function)
    {
        ctx->ReportError("Invalid drain callback function id (%X)", static_cast<int>(funcID));
        return 0;
    }

    SourceMod::IChangeableForward *forward = forwards->CreateForwardEx(nullptr, ET_Ignore, 9, nullptr,
                                                                       Param_String, Param_Cell, Param_String,
                                                                       Param_String, Param_Cell, Param_String,
                                                                       Param_Array, Param_Array, Param_Cell);
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

    realSink->drain([&forward, &data](const log4sp::details::log_msg_buffer &log_msg) {
        int64_t sec = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::seconds>(log_msg.time.time_since_epoch()).count());
        int64_t ns  = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(log_msg.time.time_since_epoch()).count());

        // See SMCore::GetTime
        cell_t pSec[]{static_cast<cell_t>(sec & 0xFFFFFFFF), static_cast<cell_t>((sec >> 32) & 0xFFFFFFFF)};
        cell_t pNs[]{static_cast<cell_t>(ns & 0xFFFFFFFF), static_cast<cell_t>((ns >> 32) & 0xFFFFFFFF)};

        forward->PushString(log_msg.logger_name.data());
        forward->PushCell(log_msg.level);
        forward->PushString(log_msg.payload.data());

        forward->PushString(log_msg.source.filename);
        forward->PushCell(log_msg.source.line);
        forward->PushString(log_msg.source.funcname);

        forward->PushArray(pSec, sizeof(pSec));
        forward->PushArray(pNs, sizeof(pNs));
        forward->PushCell(data);
        forward->Execute();
    });

    forwards->ReleaseForward(forward);
    return 0;
}

/**
 * public native void DrainFormatted(DrainFormattedCallback callback, any data = 0);
 *
 * function void (const char[] msg, any data);
 */
static cell_t RingBufferSink_DrainFormatted(SourcePawn::IPluginContext *ctx, const cell_t *params)
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

    auto realSink = std::dynamic_pointer_cast<log4sp::sinks::ringbuffer_sink>(sink);
    if (!realSink)
    {
        ctx->ReportError("Invalid ring buffer sink handle %x (error: %d)", handle, SourceMod::HandleError::HandleError_Parameter);
        return 0;
    }

    auto funcID   = static_cast<funcid_t>(params[2]);
    auto function = ctx->GetFunctionById(funcID);
    if (!function)
    {
        ctx->ReportError("Invalid drain formatted callback function id (%X)", static_cast<int>(funcID));
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

    realSink->drain_formatted([&forward, &data](std::string_view msg) {
        forward->PushString(msg.data());
        forward->PushCell(data);
        forward->Execute();
    });

    forwards->ReleaseForward(forward);
    return 0;
}

/**
 * public native int ToPattern(char[] buffer, int maxlen,
 *      const char[] name, LogLevel lvl, const char[] msg,
 *      const char[] file = NULL_STRING, int line = 0, const char[] func = NULL_STRING,
 *      int seconds[2] = {0, 0}, int nanoseconds[2] = {0, 0});
 */
static cell_t RingBufferSink_ToPattern(SourcePawn::IPluginContext *ctx, const cell_t *params)
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

    auto realSink = std::dynamic_pointer_cast<log4sp::sinks::ringbuffer_sink>(sink);
    if (!realSink)
    {
        ctx->ReportError("Invalid ring buffer sink handle %x (error: %d)", handle, SourceMod::HandleError::HandleError_Parameter);
        return 0;
    }

    char *name, *msg;
    ctx->LocalToString(params[4], &name);
    ctx->LocalToString(params[6], &msg);

    auto lvl = log4sp::level::from_number(static_cast<uint32_t>(params[5]));

    char *file, *func;
    ctx->LocalToStringNULL(params[7], &file);
    ctx->LocalToStringNULL(params[9], &func);

    auto line = static_cast<uint32_t>(params[8]);

    log4sp::source_loc loc{file, line, func};

    cell_t *seconds, *nanoseconds;
    ctx->LocalToPhysAddr(params[10], &seconds);
    ctx->LocalToPhysAddr(params[11], &nanoseconds);

    std::chrono::system_clock::time_point logTime{log4sp::details::os::now()};
    if (nanoseconds[0] != 0 || nanoseconds[1] != 0)
    {
        logTime = std::chrono::system_clock::time_point{
            std::chrono::duration_cast<std::chrono::system_clock::duration>(
                std::chrono::nanoseconds{
                    log4sp::int32_to_int64(static_cast<uint32_t>(nanoseconds[1]),
                                           static_cast<uint32_t>(nanoseconds[0]))})};
    }
    else if (seconds[0] != 0 || seconds[1] != 0)
    {
        logTime = std::chrono::system_clock::time_point{
            std::chrono::duration_cast<std::chrono::system_clock::duration>(
                std::chrono::seconds{
                    log4sp::int32_to_int64(static_cast<uint32_t>(seconds[1]),
                                           static_cast<uint32_t>(seconds[0]))})};
    }

    std::string formatted;
    try
    {
        formatted = realSink->to_pattern(log4sp::details::log_msg{logTime, loc, name, lvl, msg});
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

/**
 * public static native Logger CreateLogger(const char[] name, int amount);
 */
static cell_t RingBufferSink_CreateLogger(SourcePawn::IPluginContext *ctx, const cell_t *params)
{
    char *name;
    ctx->LocalToString(params[1], &name);
    if (log4sp::logger_handler::instance().find_handle(name) != BAD_HANDLE)
    {
        ctx->ReportError("Logger with name \"%s\" already exists.", name);
        return BAD_HANDLE;
    }

    auto amount = static_cast<size_t>(params[2]);

    log4sp::sink_ptr sink = std::make_shared<log4sp::sinks::ringbuffer_sink>(amount);

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
    {"RingBufferSink.ToPattern",                    RingBufferSink_ToPattern},
    {"RingBufferSink.CreateLogger",                 RingBufferSink_CreateLogger},

    {nullptr,                                       nullptr}
};

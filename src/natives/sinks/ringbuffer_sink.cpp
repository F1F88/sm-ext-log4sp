#include "log4sp/sinks/ringbuffer_sink.h"

#include "log4sp/adapter/sink_hanlder.h"

#include "log4sp/utils.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 RingBufferSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native RingBufferSink(int amount);
 */
static cell_t RingBufferSink(IPluginContext *ctx, const cell_t *params)
{
    auto amount = static_cast<size_t>(params[1]);
    spdlog::sink_ptr sink = std::make_shared<log4sp::sinks::ringbuffer_sink>(amount);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    Handle_t handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
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
static cell_t Drain(IPluginContext *ctx, const cell_t *params)
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

    auto realSink = std::dynamic_pointer_cast<log4sp::sinks::ringbuffer_sink>(sink);
    if (realSink == nullptr)
    {
        ctx->ReportError("Invalid ring buffer sink handle %x (error: %d)", handle, HandleError_Parameter);
        return 0;
    }

    auto funcID   = static_cast<funcid_t>(params[2]);
    auto function = ctx->GetFunctionById(funcID);
    if (!function)
    {
        ctx->ReportError("Invalid drain callback function id (%X)", static_cast<int>(funcID));
        return 0;
    }

    IChangeableForward *forward = forwards->CreateForwardEx(nullptr, ET_Ignore, 9, nullptr,
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

    realSink->drain([&forward, &data](const spdlog::details::log_msg_buffer &log_msg) {
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
static cell_t DrainFormatted(IPluginContext *ctx, const cell_t *params)
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

    auto realSink = std::dynamic_pointer_cast<log4sp::sinks::ringbuffer_sink>(sink);
    if (realSink == nullptr)
    {
        ctx->ReportError("Invalid ring buffer sink handle %x (error: %d)", handle, HandleError_Parameter);
        return 0;
    }

    auto funcID   = static_cast<funcid_t>(params[2]);
    auto function = ctx->GetFunctionById(funcID);
    if (!function)
    {
        ctx->ReportError("Invalid drain formatted callback function id (%X)", static_cast<int>(funcID));
        return 0;
    }

    IChangeableForward *forward = forwards->CreateForwardEx(nullptr, ET_Ignore, 2, nullptr, Param_String, Param_Cell);
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

    auto realSink = std::dynamic_pointer_cast<log4sp::sinks::ringbuffer_sink>(sink);
    if (realSink == nullptr)
    {
        ctx->ReportError("Invalid ring buffer sink handle %x (error: %d)", handle, HandleError_Parameter);
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

const sp_nativeinfo_t RingBufferSinkNatives[] =
{
    {"RingBufferSink.RingBufferSink",               RingBufferSink},
    {"RingBufferSink.Drain",                        Drain},
    {"RingBufferSink.DrainFormatted",               DrainFormatted},
    {"RingBufferSink.ToPattern",                    ToPattern},

    {nullptr,                                       nullptr}
};

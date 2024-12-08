#include "log4sp/sinks/client_chat_sink.h"

#include "log4sp/adapter/sink_hanlder.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 ClientChatSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native ClientChatSink(bool multiThread = false);
 */
static cell_t ClientChatSink(IPluginContext *ctx, const cell_t *params)
{
    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    bool multiThread = static_cast<bool>(params[1]);
    if (!multiThread)
    {
        auto sink   = std::make_shared<log4sp::sinks::client_chat_sink_st>();
        auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
        if (handle == BAD_HANDLE)
        {
            ctx->ReportError("Allocation of ClientChatSink handle failed. (err: %d)", handle, error);
            return BAD_HANDLE;
        }

        return handle;
    }
    else
    {
        auto sink   = std::make_shared<log4sp::sinks::client_chat_sink_mt>();
        auto handle = log4sp::sink_handler::instance().create_handle(sink, &security, nullptr, &error);
        if (handle == BAD_HANDLE)
        {
            ctx->ReportError("Allocation of multi thread ClientChatSink handle failed. (err: %d)", handle, error);
            return BAD_HANDLE;
        }

        return handle;
    }
}

/**
 * public native void SetFilter(SinkClientFilter filter);
 *
 * function Action (int client, const char[] name, LogLevel lvl, const char[] msg);
 */
static cell_t ClientChatSink_SetFilter(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);

    HandleSecurity security{nullptr, myself->GetIdentity()};
    HandleError error;

    spdlog::sink_ptr sink = log4sp::sink_handler::instance().read_handle(handle, &security, &error);
    if (sink == nullptr)
    {
        ctx->ReportError("Invalid sink handle. (hdl: %d, err: %d)", handle, error);
        return 0;
    }

    auto funcId   = static_cast<funcid_t>(params[2]);
    auto function = ctx->GetFunctionById(funcId);
    if (function == nullptr)
    {
        ctx->ReportError("Invalid SinkClientFilter function. (funcId: %d)", funcId);
        return 0;
    }

    if (typeid(sink.get()) == typeid(log4sp::sinks::client_chat_sink_st))
    {
        auto chatSink = std::dynamic_pointer_cast<log4sp::sinks::client_chat_sink_st>(sink);
        if (chatSink == nullptr)
        {
            ctx->ReportError("Unable to cast sink to single thread client_chat_sink_st.");
            return 0;
        }

        if (!chatSink->set_player_filter(function))
        {
            ctx->ReportError("SM error! Adding filter to ClientChatSink failed.");
            return 0;
        }
    }
    else
    {
        auto chatSink = std::dynamic_pointer_cast<log4sp::sinks::client_chat_sink_mt>(sink);
        if (chatSink == nullptr)
        {
            ctx->ReportError("Unable to cast sink to multi thread client_chat_sink.");
            return 0;
        }

        if (!chatSink->set_player_filter(function))
        {
            ctx->ReportError("SM error! Adding filter to multi thread ClientChatSink failed.");
            return 0;
        }
    }

    return 0;
}

const sp_nativeinfo_t ClientChatSinkNatives[] =
{
    {"ClientChatSink.ClientChatSink",               ClientChatSink},
    {"ClientChatSink.SetFilter",                    ClientChatSink_SetFilter},

    {nullptr,                                       nullptr}
};

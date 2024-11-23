#include <log4sp/client_chat_sink.h>
#include <log4sp/sink_handle_manager.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
// *                                 ClientChatSink Functions
///////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * public native ClientChatSink(bool async = false);
 */
static cell_t ClientChatSink(IPluginContext *ctx, const cell_t *params)
{
    bool async = static_cast<bool>(params[1]);
    auto data = async ? log4sp::sink_handle_manager::instance().create_client_chat_sink_st(ctx) :
                        log4sp::sink_handle_manager::instance().create_client_chat_sink_mt(ctx);

    if (data == nullptr)
    {
        return BAD_HANDLE;
    }

    return data->handle();
}

template <typename Mutex>
static void SetFilter(IPluginContext *ctx, const cell_t *params, log4sp::sink_handle_data *data)
{
    auto sink = std::dynamic_pointer_cast<log4sp::sinks::client_chat_sink<Mutex>>(data->sink_ptr());
    if (sink == nullptr)
    {
        ctx->ReportError("Unable to cast sink to client_chat_sink_%ct.", data->is_multi_threaded() ? 'm' : 's');
        return;
    }

    auto funcId = static_cast<funcid_t>(params[2]);
    auto filter = ctx->GetFunctionById(funcId);
    if (filter == NULL)
    {
        ctx->ReportError("Invalid client chat sink filter. (%d)", funcId);
        return;
    }

    if (!sink->set_player_filter(filter))
    {
        ctx->ReportError("Sets client chat sink filter failed. (%d)", funcId);
        return;
    }
}

/**
 * public native void SetFilter(ClientChatSinkFilter filter);
 */
static cell_t ClientChatSink_SetFilter(IPluginContext *ctx, const cell_t *params)
{
    auto handle = static_cast<Handle_t>(params[1]);
    auto sink = log4sp::sink_handle_manager::instance().read_handle(ctx, handle);
    if (sink == nullptr)
    {
        return 0;
    }

    auto data = log4sp::sink_handle_manager::instance().get_data(sink);
    if (data == nullptr)
    {
        ctx->ReportError("Fatal internal error, sink data not found. (hdl=%X)", static_cast<int>(handle));
        return 0;
    }

    if (!data->is_multi_threaded())
    {
        SetFilter<spdlog::details::null_mutex>(ctx, params, data);
        return 0;
    }

    SetFilter<std::mutex>(ctx, params, data);
    return 0;
}

const sp_nativeinfo_t ClientConsoleSinkNatives[] =
{
    {"ClientChatSink.ClientChatSink",               ClientChatSink},
    {"ClientChatSink.SetFilter",                    ClientChatSink_SetFilter},

    {NULL,                                          NULL}
};

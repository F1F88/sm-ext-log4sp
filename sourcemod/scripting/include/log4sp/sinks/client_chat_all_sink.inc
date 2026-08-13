#if defined _log4sp_sinks_client_chat_all_sink_included
 #endinput
#endif
#define _log4sp_sinks_client_chat_all_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/logger>
#include <log4sp/sinks/callback_sink>


methodmap ClientChatAllSink < CallbackSink
{
    /**
     * This sink is output to all client chat.
     *
     * @return          A new ClientChatAllSink Handle.
     */
    public ClientChatAllSink() {
        return view_as<ClientChatAllSink>(new CallbackSink(_, __CBSink_PrintToClientChatAll));
    }
}

static void __CBSink_PrintToClientChatAll(const char[] msg)
{
    PrintToChatAll(msg);
}

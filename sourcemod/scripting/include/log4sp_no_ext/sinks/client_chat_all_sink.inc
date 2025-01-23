#if defined _log_for_sourcepawn_sinks_client_chat_all_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_client_chat_all_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp_no_ext/sinks/callback_sink>


methodmap ClientChatAllSink < CallbackSink
{
    public ClientChatAllSink() {
        return null;
    }

    public static Logger CreateLogger(const char[] name) {
        return null;
    }
}

// static void __CBSink_PrintToClientChatAll(const char[] msg)
// {
//     PrintToChatAll(msg);
// }

#if defined _log_for_sourcepawn_sinks_client_chat_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_client_chat_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp_no_ext/sinks/sink>


methodmap ClientChatSink < Sink
{
    public ClientChatSink(SinkClientFilter filter = INVALID_FUNCTION, bool multiThread = false) {
        return view_as<ClientChatSink>(INVALID_HANDLE);
    }

    public void SetFilter(SinkClientFilter filter) {}
}

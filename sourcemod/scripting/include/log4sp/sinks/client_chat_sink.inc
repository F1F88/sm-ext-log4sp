#if defined _log_for_sourcepawn_sinks_client_chat_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_client_chat_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>


methodmap ClientChatSink < Sink
{
    /**
     * This sink is output to all client chat.
     *
     * @return              ClientChatSink handle.
     */
    public native ClientChatSink(SinkClientFilter filter = INVALID_FUNCTION);

    /**
     * Set the clint chat sink filter.
     *
     * @param filter        Filter function called when logging a message to a client.
     * @error               Invalid sink client filter.
     */
    public native void SetFilter(SinkClientFilter filter);
}

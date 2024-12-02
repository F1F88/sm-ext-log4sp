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
     * @param multiThread   If true, the created sink will be multi threaded and thread safe.
     * @return              ClientChatSink handle.
     */
    public native ClientChatSink(bool multiThread = false);

    /**
     * Set the clint chat sink filter.
     *
     * @error           Invalid client chat sink filter.
     */
    public native void SetFilter(SinkClientFilter filter);
}

#if defined _log_for_sourcepawn_sinks_client_console_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_client_console_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>


methodmap ClientConsoleSink < Sink
{
    /**
     * This sink is output to all client console.
     *
     * @param filter        Filter function called when logging a message to a client.
     *                      If INVALID_FUNCTION then no filtering, logging to all clients.
     * @param multiThread   If true, the created sink will be multi threaded and thread safe.
     * @return              ClientConsoleSink handle.
     */
    public native ClientConsoleSink(SinkClientFilter filter = INVALID_FUNCTION, bool multiThread = false);

    /**
     * Set the clint console sink filter.
     *
     * @param filter        Filter function called when logging a message to a client.
     * @error               Invalid sink client filter.
     */
    public native void SetFilter(SinkClientFilter filter);
}

#if defined _log_for_sourcepawn_sinks_server_console_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_server_console_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>


methodmap ServerConsoleSinkST < Sink
{
    /**
     * This sink is output to the server console.
     *
     * @return          Handle for a single-threaded server console sink.
     * @error           Create sink handle failed.
     */
    public native ServerConsoleSinkST();
}

methodmap ServerConsoleSinkMT < Sink
{
    /**
     * This sink is output to the server console.
     *
     * @return          Handle for a multi-threaded server console sink.
     * @error           Create sink handle failed.
     */
    public native ServerConsoleSinkMT();
}

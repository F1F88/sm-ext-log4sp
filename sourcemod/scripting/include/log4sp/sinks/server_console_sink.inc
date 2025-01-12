#if defined _log_for_sourcepawn_sinks_server_console_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_server_console_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>


methodmap ServerConsoleSink < Sink
{
    /**
     * This sink is output to the server console.
     *
     * @return              ServerConsoleSink handle.
     */
    public native ServerConsoleSink();
}

#if defined _log4sp_sinks_server_console_sink_included
 #endinput
#endif
#define _log4sp_sinks_server_console_sink_included

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

    /**
     * Create a logger handle that outputs to the server console.
     *
     * @param name      The name of the new logger.
     * @return          Logger handle.
     * @error           Logger name already exits.
     */
    public static native Logger CreateLogger(const char[] name);
}

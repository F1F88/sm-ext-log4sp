#if defined _log4sp_sinks_server_console_sink_included
    #endinput
#endif
#define _log4sp_sinks_server_console_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/logger>
#include <log4sp/sinks/sink>


/**
 * Outputs log messages to the terminal or console, useful for real-time
 * debugging and development.
 */
methodmap ServerConsoleSink < Sink
{
    /**
     * This sink is output to the server console.
     *
     * @note ServerConsoleSink handles must be freed via delete or CloseHandle().
     *
     * @return          A new ServerConsoleSink Handle.
     */
    public native ServerConsoleSink();

    /**
     * Returns whether or not a server console sink handle is valid.
     *
     * @return          True if valid, false otherwise.
     */
    public native bool IsValid();
}

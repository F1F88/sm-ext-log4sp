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
     * Create a logger handle that outputs to the server console.
     *
     * @note Logger handles must be freed via delete or CloseHandle().
     *
     * @param name      The name of the new logger.
     * @return          A new Logger Handle.
     * @error           Logger name already exists.
     */
    public static native Logger CreateLogger(const char[] name);
}

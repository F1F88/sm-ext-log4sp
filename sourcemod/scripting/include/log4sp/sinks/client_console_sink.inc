#if defined _log_for_sourcepawn_sinks_client_console_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_client_console_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>

/**
 * The filter is called at client_console_sink::sink_it_.
 * filter return Plugin_Continue to allow sink to the client console,
 * others skip sink to the client console.
 *
 * extension only call filter when the client in game,
 * so you don't need this code: "if (IsClientInGame(client)) ".
 *
 * By default (no filter is set), the sink will sink to all in game clients.
 *
 * Param msg is a message that has been formatted but not yet formatted by a pattern.
 *
 * For example, only sink to alive client
 *      Action FilterAlivePlayer(int client) {
 *          return IsPlayerAlive(client) ? Plugin_Continue : Plugin_Handled;
 *      }
 */
typeset ClientConsoleSinkFilter
{
    function Action (int client);
    function Action (int client, const char[] name, LogLevel lvl);
    function Action (int client, const char[] name, LogLevel lvl, const char[] msg);
};



methodmap ClientConsoleSink < Sink
{
    /**
     * This sink is output to all client console.
     *
     * @param async     If true, the sink will be multi threaded.
     * @return          Handle for a client console sink.
     */
    public native ClientConsoleSink(bool async = false);

    /**
     * Set the clint console sink filter.
     *
     * @error           Invalid client console sink filter.
     */
    public native void SetFilter(ClientConsoleSinkFilter filter);
}

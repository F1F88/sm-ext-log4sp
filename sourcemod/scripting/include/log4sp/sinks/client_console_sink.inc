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
 * so you don't need if (!IsClientInGame(client)).
 *
 * By default (no filter is set), the sink will sink to all in game clients.
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



methodmap ClientConsoleSinkST < Sink
{
    public native ClientConsoleSinkST();

    /**
     * Set the sink filter.
     *
     * @note 重复调用会替换旧的 filter
     *
     * @error           Invalid sink handle, invalid filter, set filter failed.
     */
    public native void SetFilter(ClientConsoleSinkFilter filter);
}

methodmap ClientConsoleSinkMT < Sink
{
    public native ClientConsoleSinkMT();

    /**
     * Set the sink filter.
     *
     * @note 重复调用会替换旧的 filter
     *
     * @error           Invalid sink handle, invalid filter, set filter failed.
     */
    public native void SetFilter(ClientConsoleSinkFilter filter);
}

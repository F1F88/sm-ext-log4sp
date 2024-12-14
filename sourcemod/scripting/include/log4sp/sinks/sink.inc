#if defined _log_for_sourcepawn_sinks_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/common>


/**
 * Base Sink
 */
methodmap Sink < Handle
{
    /**
     * Gets the sink log level.
     *
     * @return          The sink log level.
     */
    public native LogLevel GetLevel();

    /**
     * Sets the sink log level.
     *
     * @param lvl       Log level enum.
     */
    public native void SetLevel(LogLevel lvl);

    /**
     * Change log pattern.
     *
     * @note pattern flags: https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags
     *
     * @param pattern   Logger sinks pattern.
     * @param type      Pattern time.
     */
    public native void SetPattern(const char[] pattern);

    /**
     * Gets whether logging is enabled for the given log level.
     *
     * @param lvl       Log level enum.
     * @return          True if logging is enabled for the given log level.
     */
    public native bool ShouldLog(LogLevel lvl);

    /**
     * Logs a message.
     *
     * @param name      The name of a logger.
     * @param lvl       Log level enum.
     * @param msg       Log message.
     */
    public native void Log(const char[] name, LogLevel lvl, const char[] msg);

    /**
     * Manual flush logger contents.
     */
    public native void Flush();
}


/**
 * The filter is called at sink::sink_it_.
 * filter return Plugin_Continue to allow sink it, others skip sink it to this client.
 *
 * extension only call filter when the client in game, so you don't need this code: "if (IsClientInGame(client)) ".
 * By default (no filter is set), the sink will sink to all in game clients.
 *
 * Param msg is a message that has been formatted but not yet formatted by a pattern.
 *
 * For example, only sink to alive client
 *      Action FilterAlivePlayer(int client) {
 *          return IsPlayerAlive(client) ? Plugin_Continue : Plugin_Handled;
 *      }
 */
typeset SinkClientFilter
{
    function Action (int client);
    function Action (int client, const char[] name, LogLevel lvl);
    function Action (int client, const char[] name, LogLevel lvl, const char[] msg);
};

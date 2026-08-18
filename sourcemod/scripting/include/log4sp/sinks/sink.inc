#if defined _log4sp_sinks_sink_included
 #endinput
#endif
#define _log4sp_sinks_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/common>


/**
 * The Sink is the core component of the logging library responsible for the
 * actual output of log messages. Each Sink corresponds to a single log output
 * target, such as a file or the console.
 */
methodmap Sink < Handle
{
    /**
     * Logs a message.
     *
     * @param logTime   Log message timestamps (in nanoseconds).
     *                  If invalid, defaults to the current time.
     * @param loc       Source location.
     * @param name      The name of the logger.
     * @param lvl       Log level enum.
     * @param msg       Pre formatted log message.
     * @param error     Buffer to store error description.
     * @param maxlen    Maximum length of the error.
     * @return          True on success, false on error.
     */
    public native bool Log(const char[] logTime, const SourceLoc loc,
                           const char[] name, LogLevel lvl, const char[] msg,
                           char[] error="", int maxlen=0);

    /**
     * Manual flush sink contents.
     *
     * @param error     Buffer to store error description.
     * @param maxlen    Maximum length of the error.
     * @return          True on success, false on error.
     */
    public native bool Flush(char[] error="", int maxlen=0);

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
     * @note pattern flags: https://github.com/gabime/spdlog/wiki/Custom-formatting#pattern-flags
     *
     * @param pattern   Logger sinks pattern.
     * @param type      Pattern time.
     */
    public native void SetPattern(const char[] pattern, PatternTimeType type=PatternTimeType_Local);

    /**
     * Gets whether logging is enabled for the given log level.
     *
     * @param lvl       Log level enum.
     * @return          True if logging is enabled for the given log level.
     */
    public native bool ShouldLog(LogLevel lvl);

    /**
     * Compares this Sink with the specified sink for equality.
     *
     * @param sink      Sink to which this sink is to be compared.
     * @return          True if the sinks are equal.
     */
    public native bool Equals(Sink sink);
}

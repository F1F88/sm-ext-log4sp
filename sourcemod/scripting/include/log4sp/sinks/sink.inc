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
     * @param name          The name of a logger.
     * @param lvl           Log level enum.
     * @param msg           Pre formatted log message.
     * @param file          The file name of the source location.
     * @param line          The line number of the source location.
     * @param func          The function name of the source location.
     * @param seconds       The log message timestamp in seconds. Only used when not 0 and nanoseconds is 0.
     * @param nanoseconds   The log message timestamp in nanoseconds. Only used when not 0.
     */
    public native void Log(const char[] name, LogLevel lvl, const char[] msg,
        const char[] file = NULL_STRING, int line = 0, const char[] func = NULL_STRING,
        int seconds[2] = {0, 0}, int nanoseconds[2] = {0, 0});

    /**
     * Format log message in a pre-set pattern.
     *
     * @note If file is NULL_STRING, the source location will not be formatted.
     * @note If line <= 0, the source location will not be formatted.
     * @note If func is NULL_STRING, the source location will not be formatted.
     * @note If nanoseconds is not 0, nanoseconds is used as the time of the log message.
     * @note If nanoseconds is 0 and seconds is not 0, seconds is used as the time of the log message.
     * @note If nanoseconds is 0 amd seconds is 0, the current timestamp is used as the time of the log message.
     *
     * @param buffer        Buffer to store logger name.
     * @param maxlen        Maximum length of the buffer.
     * @param name          The name of a logger.
     * @param lvl           Log level enum.
     * @param msg           Pre formatted log message.
     * @param file          The file name of the source location.
     * @param line          The line number of the source location.
     * @param func          The function name of the source location.
     * @param seconds       The log message timestamp in seconds. Only used when not 0 and nanoseconds is 0.
     * @param nanoseconds   The log message timestamp in nanoseconds. Only used when not 0.
     * @return              Number of characters written to the buffer, not including the null terminator.
     */
    public native int ToPattern(char[] buffer, int maxlen,
        const char[] name, LogLevel lvl, const char[] msg,
        const char[] file = NULL_STRING, int line = 0, const char[] func = NULL_STRING,
        int seconds[2] = {0, 0}, int nanoseconds[2] = {0, 0});

    /**
     * Manual flush sink contents.
     */
    public native void Flush();
}

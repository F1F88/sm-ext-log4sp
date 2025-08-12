#if defined _log4sp_sinks_sink_included
 #endinput
#endif
#define _log4sp_sinks_sink_included

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
     * Sets formatting pattern.
     *
     * @note pattern flags: https://github.com/gabime/spdlog/wiki/Custom-formatting#pattern-flags
     *
     * @param pattern   Formatting pattern.
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
     * @param msg       Pre formatted log message.
     * @param file      The file name of the source location.
     * @param line      The line number of the source location.
     * @param func      The function name of the source location.
     * @param logTime   The log message time point in seconds.
     *                  If -1, current time point is used as the time of the log message.
     * @param error     Buffer to store error description.
     * @param maxlen    Maximum length of the error.
     * @param bytes     Number of characters written to the error, not including the null terminator.
     * @return          True on success, false on failure.
     */
    public native bool Log(const char[] name, LogLevel lvl, const char[] msg,
        const char[] file = NULL_STRING, int line = 0, const char[] func = NULL_STRING,
        int logTime = -1,
        char[] error = "", int maxlen = 0, int &bytes = 0);

    /**
     * Format log message in a pre-set pattern.
     *
     * @param buffer    Buffer to store logger name.
     * @param maxlen    Maximum length of the buffer.
     * @param lvl       Log level enum.
     * @param msg       Pre formatted log message.
     * @param file      The file name of the source location.
     * @param line      The line number of the source location.
     * @param func      The function name of the source location.
     * @param logTime   The log message time point in seconds.
     *                  If -1, current time point is used as the time of the log message.
     * @return          Number of characters written to the buffer, not including the null terminator.
     */
    public native int ToPattern(char[] buffer, int maxlen,
        const char[] name, LogLevel lvl, const char[] msg,
        const char[] file = NULL_STRING, int line = 0, const char[] func = NULL_STRING,
        int logTime = -1);

    /**
     * Manual flush sink contents.
     *
     * @param error     Buffer to store error description.
     * @param maxlen    Maximum length of the error.
     * @param bytes     Number of characters written to the error, not including the null terminator.
     * @return          True on success, false on failure.
     */
    public native bool Flush(char[] error = "", int maxlen = 0, int &bytes = 0);

    /**
     * Bind the underlying Sink object to a new Handle.
     *
     * @return              Sink handle, or BAD_HANDLE on failure.
     */
    public native Sink Clone();
}

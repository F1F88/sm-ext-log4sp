#if defined _log4sp_sinks_sink_included
 #endinput
#endif
#define _log4sp_sinks_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/common>


/**
 * Called before the sink open the file.
 */
typedef SinkFileOpenPre = function void (const char[] filename);

/**
 * Called after the sink close the file.
 */
typedef SinkFileClosePost = function void (const char[] filename);


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
     * @param msg       Pre formatted log message.
     * @param file      The file name of the source location.
     * @param line      The line number of the source location.
     * @param func      The function name of the source location.
     * @param logTime   The log message time point in seconds.
     *                  If -1, current time point is used as the time of the log message.
     */
    public native void Log(const char[] name, LogLevel lvl, const char[] msg,
        const char[] file = NULL_STRING, int line = 0, const char[] func = NULL_STRING,
        int logTime = -1);

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
     */
    public native void Flush();
}

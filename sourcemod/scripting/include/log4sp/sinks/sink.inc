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
     * Get the current logging level.
     *
     * @return          The current logging level.
     */
    public native LogLevel GetLevel();

    /**
     * Set the logging level.
     *
     * @param lvl       Logging level enum.
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
     * Gets whether logging at the given log level is enabled.
     *
     * @param lvl       Logging level enum.
     * @return          True if logging is enabled for the given level.
     */
    public native bool ShouldLog(LogLevel lvl);

    /**
     * Logging a Message.
     *
     * @param name      The name of the logger.
     * @param lvl       Logging level enum.
     * @param msg       Logging message.
     */
    public native void Log(const char[] name, LogLevel lvl, const char[] msg);

    /**
     * Logging a message with the sourcepawn source code location.
     *
     * @param name      The name of the logger.
     * @param lvl       Logging level enum.
     * @param msg       Logging message.
     */
    public native void LogSrc(const char[] name, LogLevel lvl, const char[] msg);

    /**
     * Logging a message with the source code location.
     *
     * @param file      Source code location File name.
     * @param line      Source code location File line number.
     * @param func      Source code location Function name.
     * @param name      The name of the logger.
     * @param lvl       Logging level enum.
     * @param msg       Logging message.
     */
    public native void LogLoc(const char[] file, int line, const char[] func, const char[] name, LogLevel lvl, const char[] msg);

    /**
     * Manual flush logger contents.
     */
    public native void Flush();
}

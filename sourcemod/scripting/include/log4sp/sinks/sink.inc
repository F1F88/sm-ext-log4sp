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
     * @error           Invalid sink handle.
     */
    public native LogLevel GetLevel();

    /**
     * Set the logging level.
     *
     * @note The native will log a message and:
     *       warn  - If lvl out of bounds - Fix lvl to nearest bounds.
     *
     * @param lvl       Logging level enum.
     * @error           Invalid sink handle.
     */
    public native void SetLevel(LogLevel lvl);

    /**
     * Change log pattern.
     *
     * @note pattern flags: https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags
     * @note The native will log a message and:
     *       warn  - If lvl out of bounds - Fix lvl to nearest bounds.
     *
     * @param pattern   Logger sinks pattern.
     * @param type      Pattern time.
     * @error           Invalid sink handle.
     */
    public native void SetPattern(const char[] pattern);

    /**
     * Return true logging is enabled for the given level.
     *
     * @note The native will log a message and:
     *       warn  - If lvl out of bounds - Fix lvl to nearest bounds.
     *
     * @param lvl       Logging level enum.
     * @return          True if logging is enabled for the given level.
     * @error           Invalid sink handle.
     */
    public native bool ShouldLog(LogLevel lvl);

    /**
     * Logging a Message.
     *
     * @note The native will log a message and:
     *       warn  - If lvl out of bounds - Fix lvl to nearest bounds.
     *
     * @param name      The name of the logger. This is used to identify the logger instance.
     * @param lvl       Logging level.
     * @param msg       Logging message.
     * @error           Invalid sink handle.
     */
    public native void Log(const char[] name, LogLevel lvl, const char[] msg);
    public native void LogAmxTpl(const char[] name, LogLevel lvl, const char[] fmt, any ...);
    // public native void LogFmtTpl(LogLevel lvl, const char[] fmt, any ...);

    /**
     * Logging a Message with scripted source code location.
     *
     * @note The native will log a message and:
     *       warn  - If lvl out of bounds - Fix lvl to nearest bounds.
     *
     * @param name      The name of the logger. This is used to identify the logger instance.
     * @param lvl       Logging level.
     * @param msg       Logging message.
     * @error           Invalid sink handle.
     */
    public native void LogSrc(const char[] name, LogLevel lvl, const char[] msg);
    public native void LogSrcAmxTpl(const char[] name, LogLevel lvl, const char[] fmt, any ...);
    // public native void LogSrcFmtTpl(LogLevel lvl, const char[] fmt, any ...);

    /**
     * Logging a Message with code location.
     *
     * @note The native will log a message and:
     *       warn  - If lvl out of bounds - Fix lvl to nearest bounds.
     *
     * @param file      Source code location File name.
     * @param line      Source code location File line number.
     * @param func      Source code location Function name.
     * @param name      The name of the logger. This is used to identify the logger instance.
     * @param lvl       Logging level.
     * @param msg       Logging message.
     * @error           Invalid sink handle.
     */
    public native void LogLoc(const char[] file, int line, const char[] func, const char[] name, LogLevel lvl, const char[] msg);
    public native void LogLocAmxTpl(const char[] file, int line, const char[] func, const char[] name, LogLevel lvl, const char[] fmt, any ...);
    // public native void LogLocFmtTpl(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...);

    /**
     * Manual flush logger contents.
     *
     * @error           Invalid sink handle.
     */
    public native void Flush();
}

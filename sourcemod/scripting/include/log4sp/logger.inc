#if defined _log4sp_logger_included
 #endinput
#endif
#define _log4sp_logger_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/common>
#include <log4sp/sinks/sink>


methodmap Logger < Handle
{
    /**
     * Creating a logger handle.
     *
     * @note Logger handles must be freed via delete or CloseHandle().
     *
     * @param name      The name of the new logger.
     *                  If NULL_STRING, defaults to the plugin filename.
     * @return          A new logger handle.
     */
    public native Logger(const char[] name=NULL_STRING);

    /**
     * Gets the logger name.
     *
     * @param buffer    Buffer to store logger name.
     * @param maxlen    Maximum length of the buffer.
     * @return          Number of bytes written.
     */
    public native int GetName(char[] buffer, int maxlen);

    /**
     * Gets the length of the logger name.
     *
     * @return          Length of the logger name string.
     */
    public native int GetNameLength();

    /**
     * Gets the logger log level.
     *
     * @return          The logger log level.
     */
    public native LogLevel GetLevel();

    /**
     * Sets the logger log level.
     *
     * @param lvl       Log level enum.
     */
    public native void SetLevel(LogLevel lvl);

    /**
     * Sets formatting pattern for the sinks in the logger.
     *
     * @note each sink will get a new instance of a formatter object, replacing the old one.
     * @note pattern flags: https://github.com/gabime/spdlog/wiki/Custom-formatting#pattern-flags
     *
     * @param pattern   Formatting pattern.
     * @param type      Pattern time type.
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
     * Logs a message.
     *
     * @param lvl       Log level enum.
     * @param msg       Log message.
     */
    public native void Log(LogLevel lvl, const char[] msg);
    public native void LogF(LogLevel lvl, const char[] fmt, any ...);

    /**
     * Logs a message with the scripting source code location.
     *
     * @param lvl       Log level enum.
     * @param msg       Log message.
     */
    public native void LogSrc(LogLevel lvl, const char[] msg);
    public native void LogSrcF(LogLevel lvl, const char[] fmt, any ...);

    /**
     * Logs a message with the given source location.
     *
     * @param loc       Source location.
     * @param lvl       Log level enum.
     * @param msg       Log message.
     */
    public native void LogLoc(const SourceLoc loc, LogLevel lvl, const char[] msg);
    public native void LogLocF(const SourceLoc loc, LogLevel lvl, const char[] fmt, any ...);

    /**
     * Logs a stack trace from the current function call.
     * Code execution continues after the call.
     *
     * @param lvl       Log level enum.
     * @param msg       Log message.
     */
    public native void LogStackTrace(LogLevel lvl, const char[] msg);
    public native void LogStackTraceF(LogLevel lvl, const char[] fmt, any ...);

    public native void Trace(const char[] msg);
    public native void TraceF(const char[] fmt, any ...);

    public native void Debug(const char[] msg);
    public native void DebugF(const char[] fmt, any ...);

    public native void Info(const char[] msg);
    public native void InfoF(const char[] fmt, any ...);

    public native void Warn(const char[] msg);
    public native void WarnF(const char[] fmt, any ...);

    public native void Error(const char[] msg);
    public native void ErrorF(const char[] fmt, any ...);

    public native void Fatal(const char[] msg);
    public native void FatalF(const char[] fmt, any ...);

    /**
     * Manual flush the contents for the sinks in the logger.
     */
    public native void Flush();

    /**
     * Gets the minimum log level that will trigger automatic flush.
     *
     * @return          Minimum log level that will trigger automatic flush.
     */
    public native LogLevel GetFlushLevel();

    /**
     * Sets the minimum log level that will trigger automatic flush.
     *
     * @param lvl       Log level enum.
     */
    public native void SetFlushLevel(LogLevel lvl);

    /**
     * Gets whether trigger automatic flush for the given log level.
     *
     * @param flushLvl  Log level enum.
     * @return          True if trigger automatic flush for the given log level.
     */
    public native bool ShouldFlush(LogLevel flushLvl);

    /**
     * Gets the sinks clone of logger.
     *
     * @note Sinks clone handles must be freed via delete or CloseHandle() or Sink.Close().
     *
     * @param sinks     Buffer to store sinks clone.
     * @param maxlen    Maximum length of the buffer.
     * @return          Number of sinks clone written.
     */
    public native int GetSinks(Sink[] sinks, int maxlen);

    /**
     * Gets the length of the logger sinks.
     *
     * @return          Length of the logger sinks array.
     */
    public native int GetSinksLength();

    /**
     * Add a new sink to sinks.
     *
     * @param sink      Sink handle.
     * @error           Invalid Sink handle.
     */
    public native void AddSink(Sink sink);

    /**
     * Close the cloned sink handle and remove from logger.
     *
     * @param sink      Sink handle.
     * @error           Invalid Sink handle.
     */
    public native void DropSink(Sink sink);

    /**
     * Sets the handler function for logging errors.
     *
     * @param plugin    Handle of the plugin that contains the function.
     *                  If null, defaults to the calling plugin.
     * @param func      Callback function to call when an error occurs.
     * @error           Invalid or corrupt plugin handle, or invalid function.
     */
    public native void SetErrorHandler(Handle plugin, ErrorHandler func);

    /**
     * Compares this Logger with the specified logger for equality.
     *
     * @param logger    Logger to which this logger is to be compared.
     * @return          True if the loggers are equal.
     */
    public native bool Equals(Logger logger);

    /**
     * Returns whether or not a logger handle is valid.
     *
     * @return          True if valid, false otherwise.
     */
    public native bool IsValid();
}

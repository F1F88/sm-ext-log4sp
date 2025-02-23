#if defined _log_for_sourcepawn_logger_included
 #endinput
#endif
#define _log_for_sourcepawn_logger_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/common>
#include <log4sp/sinks/sink>


/**
 * Callback for user defined error handler.
 *
 * @note By default, the handler log the error message using SourceMod API LogError().
 *
 * @note e.g.
 *      void LogToSM(const char[] msg) {
 *          LogError(msg);
 *      }
 *
 *
 * @param msg       Error message.
 * @param name      The name of the error origin.
 * @param file      The file name of the source code location.
 * @param line      The line number of the source code location.
 * @param func      The function name of the source code location.
 */
typeset LoggerErrorHandler
{
    function void (const char[] msg);
    function void (const char[] msg, const char[] name);
    function void (const char[] msg, const char[] name, const char[] file, int line, const char[] func);
};


/**
 * Callback for apply a user defined function on all logger handles.
 *
 * @note e.g.
 *      void FlushAllLoggers(Logger logger) {
 *          logger.Flush();
 *      }
 *
 * @param logger    Logger handle.
 * @param data      Data value passed to Logger.ApplyAll().
 */
typeset LoggerApplyAllCallback
{
    function void (Logger logger);
    function void (Logger logger, any data);
};


methodmap Logger < Handle
{
    /**
     * Creating a logger handle.
     *
     * @param name      The name of the new logger.
     * @return          Logger handle.
     * @error           Logger name already exits.
     */
    public native Logger(const char[] name);

    /**
     * Creating a logger handle with an array of sink handles.
     *
     * @param name      The name of the new logger.
     * @param sinks     The sinks array to be added to the new logger.
     * @param numSinks  The number of slots in the sinks array.
     * @return          Logger handle.
     * @error           Logger name already exits, or the sinks has an invalid handle.
     */
    public static native Logger CreateLoggerWith(const char[] name, Sink[] sinks, int numSinks);

    /**
     * Creating a logger handle with an array of sink handles.
     *
     * @note After creation, all sink handles in the sinks are closed.
     *
     * @param name      The name of the new logger.
     * @param sinks     The sinks array to be added to the new logger.
     * @param numSinks  The number of slots in the sinks array.
     * @return          Logger handle.
     * @error           Logger name already exits, or the sinks has an invalid handle.
     */
    public static native Logger CreateLoggerWithEx(const char[] name, Sink[] sinks, int numSinks);

    /**
     * Gets a logger handle by logger name.
     *
     * @param name      The name of the logger.
     * @return          Logger handle or INVALID_HANDLE if the logger name does not exist.
     */
    public static native Logger Get(const char[] name);

    /**
     * Apply a user defined function on all loggers.
     *
     * @param callback  A callback that apply a user defined function on all loggers.
     * @param data      Extra data value to pass to the callback.
     * @error           Invalid apply all callback.
     */
    public static native void ApplyAll(LoggerApplyAllCallback callback, any data = 0);

    /**
     * Gets the logger name.
     *
     * @param buffer    Buffer to store logger name.
     * @param maxlen    Maximum length of the buffer.
     * @return          Number of characters written to the buffer, not including the null terminator.
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
     * Sets formatting for the sinks in the logger.
     *
     * @note each sink will get a new instance of a formatter object, replacing the old one.
     * @note pattern flags: https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags
     *
     * @param pattern   Log message pattern.
     * @param type      Pattern time.
     */
    public native void SetPattern(const char[] pattern, PatternTimeType type = PatternTimeType_Local);

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
    public native void LogEx(LogLevel lvl, const char[] fmt, any ...);
    public native void LogAmxTpl(LogLevel lvl, const char[] fmt, any ...);

    /**
     * Logs a message with the sourcepawn source code location.
     *
     * @param lvl       Log level enum.
     * @param msg       Log message.
     */
    public native void LogSrc(LogLevel lvl, const char[] msg);
    public native void LogSrcEx(LogLevel lvl, const char[] fmt, any ...);
    public native void LogSrcAmxTpl(LogLevel lvl, const char[] fmt, any ...);

    /**
     * Logs a message with the given source code location.
     *
     * @param file      The file name of the source code location.
     * @param line      The line number of the source code location.
     * @param func      The function name of the source code location.
     * @param lvl       Log level enum.
     * @param msg       Log message.
     */
    public native void LogLoc(const char[] file, int line, const char[] func, LogLevel lvl, const char[] msg);
    public native void LogLocEx(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...);
    public native void LogLocAmxTpl(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...);

    /**
     * Logs a stack trace from the current function call.
     * Code execution continues after the call.
     *
     * @param lvl       Log level enum.
     * @param msg       Log message.
     */
    public native void LogStackTrace(LogLevel lvl, const char[] msg);
    public native void LogStackTraceEx(LogLevel lvl, const char[] fmt, any ...);
    public native void LogStackTraceAmxTpl(LogLevel lvl, const char[] fmt, any ...);

    /**
     * Aborts the current callback and throws an error.
     * This function does not return in that no code is executed following it.
     *
     * @note This will call SourceMod native ThrowError() first and then log the message using the logger.
     * @note A low log level does not skip the call of ThrowError()!
     * @note A low log level will skip log the message using the logger!
     *
     * @param lvl       Log level enum.
     * @param msg       Log message.
     * @error           Always!
     */
    public native void ThrowError(LogLevel lvl, const char[] msg);
    public native void ThrowErrorEx(LogLevel lvl, const char[] fmt, any ...);
    public native void ThrowErrorAmxTpl(LogLevel lvl, const char[] fmt, any ...);

    public native void Trace(const char[] msg);
    public native void TraceEx(const char[] fmt, any ...);
    public native void TraceAmxTpl(const char[] fmt, any ...);

    public native void Debug(const char[] msg);
    public native void DebugEx(const char[] fmt, any ...);
    public native void DebugAmxTpl(const char[] fmt, any ...);

    public native void Info(const char[] msg);
    public native void InfoEx(const char[] fmt, any ...);
    public native void InfoAmxTpl(const char[] fmt, any ...);

    public native void Warn(const char[] msg);
    public native void WarnEx(const char[] fmt, any ...);
    public native void WarnAmxTpl(const char[] fmt, any ...);

    public native void Error(const char[] msg);
    public native void ErrorEx(const char[] fmt, any ...);
    public native void ErrorAmxTpl(const char[] fmt, any ...);

    public native void Fatal(const char[] msg);
    public native void FatalEx(const char[] fmt, any ...);
    public native void FatalAmxTpl(const char[] fmt, any ...);

    /**
     * Manual flush the contents for the sinks in the logger.
     */
    public native void Flush();

    /**
     * Gets the minimum log level that will trigger automatic flush.
     *
     * @return          the minimum log level that will trigger automatic flush.
     */
    public native LogLevel GetFlushLevel();

    /**
     * Sets the minimum log level that will trigger automatic flush.
     *
     * @param lvl       Log level enum.
     */
    public native void FlushOn(LogLevel lvl);

    /**
     * Add a new sink to sinks.
     *
     * @param sink      Sink handle.
     * @error           Invalid sink handle.
     */
    public native void AddSink(Sink sink);

    /**
     * Add a new sink to sinks.
     *
     * @note After creation, the sink handle is closed.
     *
     * @param sink      Sink handle.
     * @error           Invalid sink handle.
     */
    public native void AddSinkEx(Sink sink);

    /**
     * Remove a sink from the sinks.
     *
     * @param sink      Sink handle.
     * @error           Invalid sink handle.
     */
    public native void DropSink(Sink sink);

    /**
     * Sets the handler function for logging errors.
     *
     * @param handler   A handler function that is called when an error occurs.
     * @error           Invalid error handler function.
     */
    public native void SetErrorHandler(LoggerErrorHandler handler);
}

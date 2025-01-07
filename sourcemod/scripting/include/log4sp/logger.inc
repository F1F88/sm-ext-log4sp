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
 * @note The default handler uses the global logger (log4sp) to log an error message
 *       containing the number of errors, the name of logger, and the error message.
 *
 * Example:
 *      void LogToSM(const char[] msg) {
 *          LogError(msg);
 *      }
 *
 * @param msg       Error message.
 */
typeset LoggerErrorHandler
{
    function void (const char[] msg);
};


/**
 * Callback for apply a user defined function on all logger handles.
 *
 * Example:
 *      void FlushAllLoggers(Logger logger) {
 *          logger.Flush();
 *      }
 *
 * @param logger    Logger handle.
 */
typeset LoggerApplyAllCallback
{
    function void (Logger logger);
};


methodmap Logger < Handle
{
    /**
     * Creating a logger handle manually.
     *
     * @param name      The name of the new logger.
     * @param sinks     The sinks array to be added to the new logger.
     * @param numSinks  The number of slots in the sinks array.
     * @param async     If true, Log operations will be pushed to background threads for processing,
     *                  which can avoid blocking the main thread.
     * @param policy    Message queue overflow policy. (Only used when async = true)
     * @return          Logger handle.
     * @error           Logger name already exits, or the sinks has an invalid handle.
     */
    public native Logger(const char[] name, Sink[] sinks, int numSinks, bool async = false, AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block);

    /**
     * Gets a logger handle by logger name.
     *
     * @param name      The name of the logger.
     * @return          Logger handle or INVALID_HANDLE if the logger name does not exist.
     */
    public static native Logger Get(const char[] name);

    /**
     * Create a logger handle that outputs to the server console.
     *
     * @param name      The name of the new logger.
     * @param async     If true, Log operations will be pushed to background threads for processing,
     *                  which can avoid blocking the main thread.
     * @param policy    Message queue overflow policy. (Only used when async = true)
     * @return          Logger handle.
     * @error           Logger name already exits.
     */
    public static native Logger CreateServerConsoleLogger(const char[] name, bool async = false, AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block);

    /**
     * Create a logger handle that outputs to a file.
     *
     * @param name      The name of the new logger.
     * @param file      The file path where the log messages will be written.
     * @param truncate  If true, the created file will be truncated.
     * @param async     If true, Log operations will be pushed to background threads for processing,
     *                  which can avoid blocking the main thread.
     * @param policy    Message queue overflow policy. (Only used when async = true)
     * @return          Logger handle.
     * @error           Logger name already exits.
     */
    public static native Logger CreateBaseFileLogger(
        const char[] name,
        const char[] file,
        bool truncate = false,
        bool async = false,
        AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block);

    /**
     * Create a logger handle that outputs to the client chat.
     *
     * @param name      The name of the new logger.
     * @param async     If true, Log operations will be pushed to background threads for processing,
     *                  which can avoid blocking the main thread.
     * @param policy    Message queue overflow policy. (Only used when async = true)
     * @return          Logger handle.
     * @error           Logger name already exits.
     */
    public static native Logger CreateClientChatLogger(const char[] name, bool async = false, AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block);

    /**
     * Create a logger handle that outputs to the client console.
     *
     * @param name      The name of the new logger.
     * @param async     If true, Log operations will be pushed to background threads for processing,
     *                  which can avoid blocking the main thread.
     * @param policy    Message queue overflow policy. (Only used when async = true)
     * @return          Logger handle.
     * @error           Logger name already exits.
     */
    public static native Logger CreateClientConsoleLogger(const char[] name, bool async = false, AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block);

    /**
     * Create a logger handle that outputs to a file and rotates the file based on size.
     *
     * @param name          The name of the logger.
     * @param file          The file path where the log messages will be written.
     * @param maxFileSize   The maximum size of the log file in bytes before it gets rotated.
     * @param maxFiles      The maximum number of rotated log files to keep.
     * @param rotateOnOpen  If true, the log file will be rotated when opened.
     * @param async         If true, Log operations will be pushed to background threads for processing,
     *                      which can avoid blocking the main thread.
     * @param policy        Message queue overflow policy. (Only used when async = true)
     * @return              Logger handle.
     * @error               Logger name already exits, or maxFileSize == 0, or maxFiles > 200000.
     */
    public static native Logger CreateRotatingFileLogger(
        const char[] name,
        const char[] file,
        int maxFileSize,
        int maxFiles,
        bool rotateOnOpen = false,
        bool async = false,
        AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block);

    /**
     * Create a logger handle that outputs to a file and rotates the file based on date.
     *
     * @param name      The name of the logger.
     * @param file      The file path where the log messages will be written.
     * @param hour      The hour of the day when the log file should be rotated. (0-23)
     * @param minute    The minute of the hour when the log file should be rotated. (0-59)
     * @param truncate  If true, the created file will be truncated.
     * @param maxFiles  If max_files > 0, retain only the last max_files and delete previous.
     * @param async     If true, Log operations will be pushed to background threads for processing,
     *                  which can avoid blocking the main thread.
     * @param policy    Message queue overflow policy. (Only used when async = true)
     * @return          Logger handle.
     * @error           Logger name already exits, or invalid rotation time.
     */
    public static native Logger CreateDailyFileLogger(
        const char[] name,
        const char[] file,
        int hour = 0,
        int minute = 0,
        bool truncate = false,
        int maxFiles = 0,
        bool async = false,
        AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block);

    /**
     * Apply a user defined function on all loggers.
     *
     * @param callback  A callback that apply a user defined function on all loggers.
     * @error           Invalid apply all callback.
     */
    public static native void ApplyAll(LoggerApplyAllCallback callback);

    /**
     * Gets the logger name.
     *
     * @param buffer    Buffer to store logger name.
     * @param maxlen    Maximum length of the buffer.
     * @return          Number of characters written to the buffer, not including the null terminator.
     */
    public native int GetName(char[] buffer, int maxlen);

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
     * @note The only difference from ThorwError() is that it also log the message into the logger's sinks.
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
     * Manual flush logger contents.
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
     * Gets whether backtrace logging is enabled.
     *
     * @return          True if backtrace logging is enabled.
     */
    public native bool ShouldBacktrace();

    /**
     * Create new backtrace sink and move to it all our child sinks.
     *
     * @param num       Number of log messages stored.
     */
    public native void EnableBacktrace(int num);

    /**
     * Restore original sinks and level and delete the backtrace sink.
     */
    public native void DisableBacktrace();

    /**
     * Dump log messages stored in the backtrace ring buffer.
     */
    public native void DumpBacktrace();

    /**
     * Add a new sink to sinks.
     *
     * @param sink      Sink handle.
     * @error           Invalid sink handle.
     */
    public native void AddSink(Sink sink);

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

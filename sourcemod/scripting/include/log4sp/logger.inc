#if defined _log_for_sourcepawn_logger_included
 #endinput
#endif
#define _log_for_sourcepawn_logger_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/common>
#include <log4sp/sinks/sink>


typeset Log4spErrorCallback
{
    function void (const char[] msg);
};


methodmap Logger < Handle
{
    /**
     * Creating logger handle manually.
     *
     * @note While single threaded sinks cannot be used from multiple threads simultaneously they might be faster because no locking is employed.
     *
     * @param name      The name of the new logger.
     * @param sinks     The sinks array to be added to the new logger.
     * @param numSinks  The number of slots in the sinks array.
     * @param async     If true, the sink of the new logger will be multi threaded.
     * @param policy    Message queue overflow policy. (Only used when async=true)
     * @return          Logger handle.
     * @error           Logger name already exits, or the sinks has an invalid handle.
     */
    public native Logger(const char[] name, Sink[] sinks, int numSinks, bool async = false, AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block);

    /**
     * Get a logger handle by logger name.
     *
     * @param name      The name of the logger.
     * @return          Return an existing logger handle or INVALID_HANDLE if a logger with such name doesn't exist.
     */
    public static native Logger Get(const char[] name);

    /**
     * Create a logger handle that outputs to the server console.
     *
     * @note While single threaded sinks cannot be used from multiple threads simultaneously they might be faster because no locking is employed.
     *
     * @param name      The name of the new logger.
     * @param async     If true, the sink of the new logger will be multi threaded.
     * @param policy    Message queue overflow policy. (Only used when async=true)
     * @return          Logger handle.
     * @error           Logger name already exits.
     */
    public static native Logger CreateServerConsoleLogger(const char[] name, bool async = false, AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block);

    /**
     * Create a logger handle that outputs to a file.
     *
     * @note While single threaded sinks cannot be used from multiple threads simultaneously they might be faster because no locking is employed.
     *
     * @param name      The name of the new logger.
     * @param file      The file path where the log messages will be written.
     * @param truncate  If true, the created file will be truncated.
     * @param async     If true, the sink of the new logger will be multi threaded.
     * @param policy    Message queue overflow policy. (Only used when async=true)
     * @return          Logger handle.
     * @error           Logger name already exits.
     */
    public static native Logger CreateBaseFileLogger(const char[] name,
                                                     const char[] file,
                                                     bool truncate = false,
                                                     bool async = false,
                                                     AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block);

    /**
     * Create a logger handle that outputs to a file and rotates the file based on size.
     *
     * Rotate files:
     *      log.txt   -> log.1.txt
     *      log.1.txt -> log.2.txt
     *      log.2.txt -> log.3.txt
     *      log.3.txt -> delete
     *
     * @note While single threaded sinks cannot be used from multiple threads simultaneously they might be faster because no locking is employed.
     *
     * @param name          The name of the logger.
     * @param file          The file path where the log messages will be written.
     * @param maxFileSize   The maximum size of the log file in bytes before it gets rotated.
     * @param maxFiles      The maximum number of rotated log files to keep.
     * @param rotateOnOpen  If true, the log file will be rotated when opened.
     * @param async         If true, the sink of the new logger will be multi threaded.
     * @param policy        Message queue overflow policy. (Only used when async=true)
     * @return              Logger handle.
     * @error               Logger name already exits, or maxFileSize == 0, or maxFiles > 200000.
     */
    public static native Logger CreateRotatingFileLogger(const char[] name,
                                                         const char[] file,
                                                         int maxFileSize,
                                                         int maxFiles,
                                                         bool rotateOnOpen = false,
                                                         bool async = false,
                                                         AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block);

    /**
     * Create a logger handle that outputs to a file and rotates the file based on date.
     *
     * @note While single threaded sinks cannot be used from multiple threads simultaneously they might be faster because no locking is employed.
     *
     * @param name      The name of the logger.
     * @param file      The file path where the log messages will be written.
     * @param hour      The hour of the day when the log file should be rotated. (0-23)
     * @param minute    The minute of the hour when the log file should be rotated. (0-59)
     * @param truncate  If true, the created file will be truncated.
     * @param maxFiles  If max_files > 0, retain only the last max_files and delete previous.
     * @param async     If true, the sink of the new logger will be multi threaded.
     * @param policy    Message queue overflow policy. (Only used when async=true)
     * @return          Logger handle.
     * @error           Logger name already exits, or invalid rotation time.
     */
    public static native Logger CreateDailyFileLogger(const char[] name,
                                                      const char[] file,
                                                      int hour = 0,
                                                      int minute = 0,
                                                      bool truncate = false,
                                                      int maxFiles = 0,
                                                      bool async = false,
                                                      AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block);

    /**
     * Get the logger name.
     *
     * @param buffer    Buffer to store logger name.
     * @param maxlen    Maximum length of the buffer.
     */
    public native void GetName(char[] buffer, int maxlen);

    /**
     * Get the logger log level.
     *
     * @return          The logger log level.
     */
    public native LogLevel GetLevel();

    /**
     * Set the logger log level.
     *
     * @note If lvl out of bounds, native will be modified to the nearest boundary.
     *
     * @param lvl       Logging level enum.
     */
    public native void SetLevel(LogLevel lvl);

    /**
     * Set formatting for the sinks in this logger.
     *
     * @note each sink will get a new instance of a formatter object, replacing the old one.
     * @note pattern flags: https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags
     *
     * @param pattern   Log message pattern.
     * @param type      Pattern time.
     */
    public native void SetPattern(const char[] pattern, PatternTimeType type = PatternTimeType_Local);

    /**
     * Gets whether logging at the given log level is enabled.
     *
     * @note If lvl out of bounds, native will be modified to the nearest boundary.
     *
     * @param lvl       Logging level enum.
     * @return          True if logging is enabled for the given level.
     */
    public native bool ShouldLog(LogLevel lvl);

    /**
     * Logs a message.
     *
     * @param lvl       Logging level enum.
     * @param msg       Logging message.
     */
    public native void Log(LogLevel lvl, const char[] msg);
    public native void LogAmxTpl(LogLevel lvl, const char[] fmt, any ...);

    /**
     * Logs a message with the sourcepawn source code location.
     *
     * @param lvl       Logging level enum.
     * @param msg       Logging message.
     */
    public native void LogSrc(LogLevel lvl, const char[] msg);
    public native void LogSrcAmxTpl(LogLevel lvl, const char[] fmt, any ...);

    /**
     * Logs a message with the source code location.
     *
     * @param file      The file name of the source code location.
     * @param line      The line number of the source code location.
     * @param func      The function name of the source code location.
     * @param lvl       Logging level enum.
     * @param msg       Logging message.
     */
    public native void LogLoc(const char[] file, int line, const char[] func, LogLevel lvl, const char[] msg);
    public native void LogLocAmxTpl(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...);

    /**
     * Logs a stack trace from the current function call.
     * Code execution continues after the call.
     *
     * @param lvl       Logging level enum.
     * @param msg       Logging message.
     */
    public native void LogStackTrace(LogLevel lvl, const char[] msg);
    public native void LogStackTraceAmxTpl(LogLevel lvl, const char[] fmt, any ...);

    /**
     * Aborts the current callback and throws an error.
     * This function does not return in that no code is executed following it.
     *
     * @note The only difference from ThorwError() is that it also log the message into the logger's sinks.
     *
     * @param lvl       Logging level enum.
     * @param msg       Logging message.
     * @error           Always!
     */
    public native void ThrowError(LogLevel lvl, const char[] msg);
    public native void ThrowErrorAmxTpl(LogLevel lvl, const char[] fmt, any ...);

    public native void Trace(const char[] msg);
    public native void TraceAmxTpl(const char[] fmt, any ...);

    public native void Debug(const char[] msg);
    public native void DebugAmxTpl(const char[] fmt, any ...);

    public native void Info(const char[] msg);
    public native void InfoAmxTpl(const char[] fmt, any ...);

    public native void Warn(const char[] msg);
    public native void WarnAmxTpl(const char[] fmt, any ...);

    public native void Error(const char[] msg);
    public native void ErrorAmxTpl(const char[] fmt, any ...);

    public native void Fatal(const char[] msg);
    public native void FatalAmxTpl(const char[] fmt, any ...);

    /**
     * Manual flush logger contents.
     */
    public native void Flush();

    /**
     * Get the minimum log level that will trigger automatic flush.
     *
     * @return          the minimum log level that will trigger automatic flush.
     */
    public native LogLevel GetFlushLevel();

    /**
     * Set the minimum log level that will trigger automatic flush.
     *
     * @note If lvl out of bounds, native will be modified to the nearest boundary.
     *
     * @param lvl       Logging level enum.
     */
    public native void FlushOn(LogLevel lvl);

    /**
     * Gets whether backtrace logging is enabled.
     *
     * @return          Return true if backtrace logging is enabled.
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
     * @note Do not use a single threaded sink in a multi threaded logger, otherwise unexpected things may happen.
     * @note This machine is not thread safe.
     *       Recommend using new Logger() or only using this native during initialization.
     *       (See: https://github.com/F1F88/sm-ext-log4sp/issues/4)
     *
     * @param sink      Handle of sink.
     * @error           Invalid sink handle.
     */
    public native void AddSink(Sink sink);

    /**
     * Drop the reference to the given sink
     *
     * @param sink      Handle of Sink.
     * @return          True on success, false if the sink was not found.
     * @error           Invalid logger handle, invalid sink handle.
     */
    public native bool DropSink(Sink sink);

    /**
     * Sets the callback when logging errors occur.
     *
     * @note This machine is not thread safe.
     *       Recommend using this native only during initialization.
     *
     * @param callback  A callback called when an error occurs.
     * @error           Invalid callback.
     */
    public native void SetErrorHandler(Log4spErrorCallback callback);
}

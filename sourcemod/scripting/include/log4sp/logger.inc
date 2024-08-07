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
     * Creating logger manually.
     *
     * @param name      Name of new logger.
     * @param sinks     The sinks array of the logger.
     * @param numSinks  Number of slots in the sinks array.
     * @param async     If true, cteate an asynchronous logger.
     * @param policy    Asynchronous queue overflow policy. Only valid when param async = true.
     * @return          Handle of Logger.
     * @error           Logger name already exits, param sinks has invalid sink handle, param policy is out of bounds, create logger handle failed.
     */
    public native Logger(const char[] name, Sink[] sinks, int numSinks, bool async = false, AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block);

    /**
     * Get a logger by logger name
     *
     * @param name      Name of logger.
     * @return          Handle of Logger.
     * @error           Logger name does not exits, create logger handle failed.
     */
    public static native Logger Get(const char[] name);

    /**
     * Server console sink base on stdout.
     *
     * @param name      Name of new logger.
     * @param async     If true, cteate an asynchronous logger.
     *                  And sink run in multi threaded. They might be slower because locking is required.
     * @param policy    Asynchronous queue overflow policy. Only valid when param async = true.
     * @return          Handle of Logger.
     * @error           Logger name already exits, invalid policy, create logger handle failed.
     */
    public static native Logger CreateServerConsoleLogger(const char[] name, bool async = false, AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block);

    /**
     * Trivial file sink with single file as target.
     *
     * @param name      The name of the logger. This is used to identify the logger instance.
     * @param file      The file path where the log messages will be written.
     * @param truncate  If truncate != false , the created file will be truncated.
     * @param async     If true, cteate an asynchronous logger.
     *                  And sink run in multi threaded. They might be slower because locking is required.
     * @param policy    Asynchronous queue overflow policy. Only valid when param async = true.
     * @return          Handle of Logger.
     * @error           Logger name already exits, invalid policy, create logger handle failed.
     */
    public static native Logger CreateBaseFileLogger(
        const char[] name,
        const char[] file,
        bool truncate = false,
        bool async = false,
        AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block
    );

    /**
     * Rotating file sink based on size.
     * Rotate files:
     *      log.txt   -> log.1.txt
     *      log.1.txt -> log.2.txt
     *      log.2.txt -> log.3.txt
     *      log.3.txt -> delete
     *
     * @param name          The name of the logger. This is used to identify the logger instance.
     * @param file          The file path where the log messages will be written.
     * @param maxFileSize   The maximum size of the log file in bytes before it gets rotated.
     * @param maxFiles      The maximum number of rotated log files to keep.
     * @param rotateOnOpen  Whether to rotate the log file upon opening.
     * @param async         If true, cteate an asynchronous logger. And sink run in multi threaded.
     *                      They might be slower because locking is required.
     * @param policy        Asynchronous queue overflow policy. Useful only if async = true.
     * @return              Handle of Logger.
     * @error               Logger name already exits, param maxFileSize <= 0, param maxFiles > 200000, param policy is out of bounds, create logger handle failed.
     */
    public static native Logger CreateRotatingFileLogger(
        const char[] name,
        const char[] file,
        int maxFileSize,
        int maxFiles,
        bool rotateOnOpen = false,
        bool async = false,
        AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block
    );

    /**
     * Rotating file sink based on date.
     * If truncate != false , the created file will be truncated.
     * If max_files > 0, retain only the last max_files and delete previous.
     *
     * @param name      The name of the logger. This is used to identify the logger instance.
     * @param file      The file path where the log messages will be written.
     * @param hour      The hour of the day when the log file should be rotated (0-23). Default is 0.
     * @param minute    The minute of the hour when the log file should be rotated (0-59). Default is 0.
     * @param truncate  If truncate != false , the created file will be truncated.
     * @param maxFiles  If max_files > 0, retain only the last max_files and delete previous.
     * @param async     If true, cteate an asynchronous logger. And sink run in multi threaded.
     *                  They might be slower because locking is required.
     * @param policy    Asynchronous queue overflow policy. Useful only if async = true.
     * @return          Handle of Logger.
     * @error           Logger name already exits, invalid rotation time in ctor, invalid policy, create logger handle failed.
     */
    public static native Logger CreateDailyFileLogger(
        const char[] name,
        const char[] file,
        int hour = 0,
        int minute = 0,
        bool truncate = false,
        int maxFiles = 0,
        bool async = false,
        AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block
    );

    /**
     * Get the current logger name.
     *
     * @param buffer    Buffer to store logger name.
     * @param maxlen    Maximum size of string buffer.
     * @error           Invalid logger handle.
     */
    public native void GetName(char[] buffer, int maxlen);

    /**
     * Get the current logging level.
     *
     * @return          The current logging level.
     * @error           Invalid logger handle.
     */
    public native LogLevel GetLevel();

    /**
     * Set the logging level.
     *
     * @note The native will log a message and:
     *       warn  - If lvl out of bounds - Fix lvl to nearest bounds.
     *
     * @param lvl       Logging level enum.
     * @error           Invalid logger handle.
     */
    public native void SetLevel(LogLevel lvl);

    /**
     * Change log pattern.
     *
     * @note pattern flags: https://github.com/gabime/spdlog/wiki/3.-Custom-formatting#pattern-flags
     *
     * @param pattern   Logger sinks pattern.
     * @param type      Pattern time.
     * @error           Invalid logger handle.
     */
    public native void SetPattern(const char[] pattern, PatternTimeType type = PatternTimeType_Local);

    /**
     * Return true logging is enabled for the given level.
     *
     * @note The native will log a message and:
     *       warn  - If lvl out of bounds - Fix lvl to nearest bounds.
     *
     * @param lvl       Logging level enum.
     * @return          True if logging is enabled for the given level.
     * @error           Invalid logger handle.
     */
    public native bool ShouldLog(LogLevel lvl);

    /**
     * Logging a Message.
     *
     * @note The native will log a message and:
     *       warn  - If lvl out of bounds - Fix lvl to nearest bounds.
     *
     * @param lvl       Logging level enum.
     * @param msg       Logging message.
     * @error           Invalid logger handle.
     */
    public native void Log(LogLevel lvl, const char[] msg);
    public native void LogAmxTpl(LogLevel lvl, const char[] fmt, any ...);
    // public native void LogFmtTpl(LogLevel lvl, const char[] fmt, any ...);

    /**
     * Logging a Message with scripted source code location.
     *
     * @note The native will log a message and:
     *       warn  - If lvl out of bounds - Fix lvl to nearest bounds.
     *
     * @param lvl       Logging level enum.
     * @param msg       Logging message.
     * @error           Invalid logger handle.
     */
    public native void LogSrc(LogLevel lvl, const char[] msg);
    public native void LogSrcAmxTpl(LogLevel lvl, const char[] fmt, any ...);
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
     * @param lvl       Logging level enum.
     * @param msg       Logging message.
     * @error           Invalid logger handle.
     */
    public native void LogLoc(const char[] file, int line, const char[] func, LogLevel lvl, const char[] msg);
    public native void LogLocAmxTpl(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...);
    // public native void LogLocFmtTpl(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...);

    public native void Trace(const char[] msg);
    public native void TraceAmxTpl(const char[] fmt, any ...);
    // public native void TraceFmtTpl(const char[] fmt, any ...);

    public native void Debug(const char[] msg);
    public native void DebugAmxTpl(const char[] fmt, any ...);
    // public native void DebugFmtTpl(const char[] fmt, any ...);

    public native void Info(const char[] msg);
    public native void InfoAmxTpl(const char[] fmt, any ...);
    // public native void InfoFmtTpl(const char[] fmt, any ...);

    public native void Warn(const char[] msg);
    public native void WarnAmxTpl(const char[] fmt, any ...);
    // public native void WarnFmtTpl(const char[] fmt, any ...);

    public native void Error(const char[] msg);
    public native void ErrorAmxTpl(const char[] fmt, any ...);
    // public native void ErrorFmtTpl(const char[] fmt, any ...);

    public native void Fatal(const char[] msg);
    public native void FatalAmxTpl(const char[] fmt, any ...);
    // public native void FatalFmtTpl(const char[] fmt, any ...);

    /**
     * Manual flush logger contents.
     *
     * @error           Invalid logger handle.
     */
    public native void Flush();

    /**
     * Get the minimum log level that will trigger automatic flush.
     *
     * @return          the minimum log level that will trigger automatic flush.
     * @error           Invalid logger handle.
     */
    public native LogLevel GetFlushLevel();

    /**
     * Set the minimum log level that will trigger automatic flush.
     *
     * @note The native will log a message and:
     *       warn  - If lvl out of bounds - Fix lvl to nearest bounds.
     *
     * @param lvl       Logging level enum.
     * @error           Invalid logger handle.
     */
    public native void FlushOn(LogLevel lvl);

    /**
     * Return true if backtrace logging is enabled.
     *
     * @return          True if backtrace logging is enabled.
     * @error           Invalid logger handle.
     */
    public native bool ShouldBacktrace();

    /**
     * Create new backtrace sink and move to it all our child sinks.
     *
     * @param num       Number of log messages stored. (type - unsigned int)
     * @error           Invalid logger handle.
     */
    public native void EnableBacktrace(int num);

    /**
     * Restore orig sinks and level and delete the backtrace sink.
     *
     * @error           Invalid logger handle.
     */
    public native void DisableBacktrace();

    /**
     * Dump the backtrace of logged messages.
     *
     * @error           Invalid logger handle.
     */
    public native void DumpBacktrace();

    /**
     * Add a new sink to sinks.
     *
     * @param sink      Handle of sink.
     * @error           Invalid logger handle, invalid sink handle.
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
     * Sets the callback when logging errors occur
     *
     * @note 还不完善
     *
     * @param callback  A function to use as a callback when the logger occur errors.
     * @error           Invalid logger handle, callback not found.
     */
    public native void SetErrorHandler(Log4spErrorCallback callback);
}

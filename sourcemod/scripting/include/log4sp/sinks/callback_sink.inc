#if defined _log_for_sourcepawn_sinks_callback_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_callback_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>


typeset CustomLogCallback
{
    /**
     * Callback function called when Sink.Log() is called.
     *
     * @param name          The name of a logger.
     * @param lvl           Log level enum.
     * @param msg           Pre formatted log message.
     * @param file          The file name of the source code location.
     * @param line          The line number of the source code location.
     * @param func          The function name of the source code location.
     * @param seconds       The log message timestamp in seconds.
     * @param nanoseconds   The log message timestamp in nanoseconds.
     */
    function void (const char[] name, LogLevel lvl, const char[] msg);
    function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func);
    function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int seconds[2]);
    function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int seconds[2], int nanoseconds[2]);

    /**
     * Callback function called after Sink.Log() is called.
     *
     * @param msg           Log messages that have been formatted in a pre-set pattern.
     */
    function void (const char[] msg);
}

/**
 * Callback function called when Sink.Flush() is called.
 */
typedef CustomFlushCallback = function void ();


/**
 * Trivial callback sink, gets a callback function and calls it on each log or flush.
 */
methodmap CallbackSink < Sink
{
    /**
     * Trivial callback sink, gets a callback function and calls it on each log or flush.
     *
     * @param logCallback       Callback function called when Sink.Log() is called.
     * @param logPostCallback   Callback function called after Sink.Log() is called.
     * @param flushCallback     Callback function called when Sink.Flush() is called.
     * @return                  CallbackSink handle.
     */
    public native CallbackSink(
        CustomLogCallback logCallback = INVALID_FUNCTION,
        CustomLogCallback logPostCallback = INVALID_FUNCTION,
        CustomFlushCallback flushCallback = INVALID_FUNCTION);

    /**
     * Set the callback sink log callback.
     *
     * @param logCallback       Callback function called when Sink.Log() is called.
     */
    public native void SetLogCallback(CustomLogCallback logCallback);

    /**
     * Set the callback sink log post callback.
     *
     * @param logPostCallback   Callback function called after Sink.Log() is called.
     */
    public native void SetLogPostCallback(CustomLogCallback logPostCallback);

    /**
     * Set the callback sink flush callback.
     *
     * @param flushCallback     Callback function called when Sink.Flush() is called.
     */
    public native void SetFlushCallback(CustomFlushCallback flushCallback);

    /**
     * Format log message in a pre-set pattern.
     *
     * @note If file is NULL_STRING, the source location will not be formatted.
     * @note If line is 0, the source location will not be formatted.
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
    public native int FormatPattern(char[] buffer, int maxlen,
        const char[] name, LogLevel lvl, const char[] msg,
        const char[] file = NULL_STRING, int line = 0, const char[] func = NULL_STRING,
        int seconds[2] = {0, 0}, int nanoseconds[2] = {0, 0});
}

#if defined _log_for_sourcepawn_sinks_callback_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_callback_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>


/**
 * Callback function called when logging a message.
 *
 * @param name          The name of a logger.
 * @param lvl           Log level enum.
 * @param msg           Log message.
 * @param file          The file name of the source code location.
 * @param line          The line number of the source code location.
 * @param func          The function name of the source code location.
 * @param seconds       The log message timestamp in seconds.
 * @param nanoseconds   The log message timestamp in nanoseconds.
 */
typeset CustomLogCallback
{
    function void(const char[] name, LogLevel lvl, const char[] msg);
    function void(const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func);
    function void(const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int seconds[2]);
    function void(const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int seconds[2], int nanoseconds[2]);
}

/**
 * Callback function called when flush.
 */
typedef CustomFlushCallback = function void();


/**
 * Trivial callback sink, gets a callback function and calls it on each log or flush.
 */
methodmap CallbackSink < Sink
{
    /**
     * Trivial callback sink, gets a callback function and calls it on each log or flush.
     *
     * @param logCallback   Callback function called when logging a message.
     * @param flushCallback Callback function called when flush.
     * @return              CallbackSink handle.
     * @error               Invalid callback sink log callback.
     */
    public native CallbackSink(CustomLogCallback logCallback, CustomFlushCallback flushCallback = INVALID_FUNCTION);

    /**
     * Set the callback sink log callback.
     *
     * @param logCallback   Callback function called when logging a message.
     * @error               Invalid callback sink log callback.
     */
    public native void SetLogCallback(CustomLogCallback logCallback);

    /**
     * Set the callback sink flush callback.
     *
     * @param flushCallback Callback function called when flush.
     * @error               Invalid callback sink flush callback.
     */
    public native void SetFlushCallback(CustomFlushCallback flushCallback);

    /**
     * TODO Docs
     *
     * @note If nanoseconds is not 0, nanoseconds is used as the time of the log message.
     * @note If nanoseconds is 0 and seconds is not 0, seconds is used as the time of the log message.
     * @note If nanoseconds and seconds are both 0, the current timestamp is used as the time of the log message.
     *
     * @param buffer        Buffer to store logger name.
     * @param maxlen        Maximum length of the buffer.
     * @param name          The name of a logger.
     * @param lvl           Log level enum.
     * @param msg           Log message.
     * @param file          The file name of the source code location.
     * @param line          The line number of the source code location.
     * @param func          The function name of the source code location.
     * @param seconds       The log message timestamp in seconds. Only used when nanoseconds is 0.
     * @param nanoseconds   The log message timestamp in nanoseconds. If 0, seconds is used.
     * @return              Number of characters written to the buffer, not including the null terminator.
     */
    public native int FormatPattern(char[] buffer, int maxlen,
        const char[] name, LogLevel lvl, const char[] msg,
        const char[] file = NULL_STRING, int line = 0, const char[] func = NULL_STRING,
        int seconds[2] = {0, 0}, int nanoseconds[2] = {0, 0});
}

#if defined _log4sp_sinks_callback_sink_included
 #endinput
#endif
#define _log4sp_sinks_callback_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>


/**
 * Callback function called when Sink.Log() is called.
 *
 * @param name          The name of a logger.
 * @param lvl           Log level enum.
 * @param msg           Pre formatted log message.
 * @param file          The file name of the source code location.
 * @param line          The line number of the source code location.
 * @param func          The function name of the source code location.
 * @param logTime       The log message time point in seconds.
 */
typeset CustomLogCallback
{
    function void (const char[] name, LogLevel lvl, const char[] msg);
    function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func);
    function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int logTime);
}


/**
 * Callback function called after Sink.Log() is called.
 *
 * @param msg           Log messages that have been formatted in a pre-set pattern.
 */
typedef CustomLogPostCallback = function void (const char[] msg);


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
    public native CallbackSink(CustomLogCallback logCallback = INVALID_FUNCTION,
        CustomLogPostCallback logPostCallback = INVALID_FUNCTION,
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
    public native void SetLogPostCallback(CustomLogPostCallback logPostCallback);

    /**
     * Set the callback sink flush callback.
     *
     * @param flushCallback     Callback function called when Sink.Flush() is called.
     */
    public native void SetFlushCallback(CustomFlushCallback flushCallback);

    /**
     * Create a logger handle that gets a callback function and calls it on each log or flush.
     *
     * @param name              The name of the new logger.
     * @param logCallback       Callback function called when Sink.Log() is called.
     * @param logPostCallback   Callback function called after Sink.Log() is called.
     * @param flushCallback     Callback function called when Sink.Flush() is called.
     * @return                  Logger handle.
     * @error                   Logger name already exits.
     */
    public static native Logger CreateLogger(const char[] name,
        CustomLogCallback logCallback = INVALID_FUNCTION,
        CustomLogPostCallback logPostCallback = INVALID_FUNCTION,
        CustomFlushCallback flushCallback = INVALID_FUNCTION);
}

#if defined _log4sp_sinks_callback_sink_included
 #endinput
#endif
#define _log4sp_sinks_callback_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/logger>
#include <log4sp/sinks/sink>


/**
 * Callback function called when Sink.Log() is called.
 *
 * @param name          The name of the logger.
 * @param lvl           Log level enum.
 * @param msg           Pre formatted log message.
 * @param file          The file name of the source code location.
 * @param line          The line number of the source code location.
 * @param func          The function name of the source code location.
 * @param logTime       Log message timestamps (in seconds).
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
     * @note CallbackSink handles must be freed via delete or CloseHandle().
     *
     * @param logCallback       Function to call when CallbackSink.Log() is called.
     * @param logPostCallback   Function to call after CallbackSink.Log() is called.
     * @param flushCallback     Function to call when CallbackSink.Flush() is called.
     * @return                  A new CallbackSink Handle.
     */
    public native CallbackSink(CustomLogCallback logCallback=INVALID_FUNCTION,
        CustomLogPostCallback logPostCallback=INVALID_FUNCTION,
        CustomFlushCallback flushCallback=INVALID_FUNCTION);

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
     * @note Logger handles must be freed via delete or CloseHandle().
     *
     * @param name              The name of the new logger.
     * @param logCallback       Function to call when CallbackSink.Log() is called.
     * @param logPostCallback   Function to call after CallbackSink.Log() is called.
     * @param flushCallback     Function to call when CallbackSink.Flush() is called.
     * @return                  A new Logger Handle.
     * @error                   Logger name already exists.
     */
    public static native Logger CreateLogger(const char[] name,
        CustomLogCallback logCallback=INVALID_FUNCTION,
        CustomLogPostCallback logPostCallback=INVALID_FUNCTION,
        CustomFlushCallback flushCallback=INVALID_FUNCTION);
}

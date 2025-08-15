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
 * @param name          The name of a logger.
 * @param lvl           Log level enum.
 * @param msg           Pre formatted log message.
 * @param file          The file name of the source code location.
 * @param line          The line number of the source code location.
 * @param func          The function name of the source code location.
 * @param logTime       The log message time point in seconds.
 * @param data          Data value passed to the CallbackSink callbacks.
 * @param error         Error message buffer in case of log failure.
 * @return              False for log success, True otherwise.
 *                      If True, an exception with error description will be thrown.
 */
typeset CustomLogCallback
{
    function void (const char[] name, LogLevel lvl, const char[] msg);
    function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func);
    function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int logTime);
    function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int logTime, any data);
    function bool (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int logTime, any data, char error[256]);
}


/**
 * Callback function called after Sink.Log() is called.
 *
 * @param msg           Log messages that have been formatted in a pre-set pattern.
 * @param data          Data value passed to the CallbackSink callbacks.
 * @param error         Error message buffer in case of log failure.
 * @return              False for log success, True otherwise.
 *                      If True, an exception with error description will be thrown.
 */
typeset CustomLogPostCallback
{
    function void (const char[] msg);
    function void (const char[] msg, any data);
    function bool (const char[] msg, any data, char error[256]);
}


/**
 * Callback function called when Sink.Flush() is called.
 *
 * @param error         Error message buffer in case of flush failure.
 * @param data          Data value passed to the CallbackSink callbacks.
 * @return              False for flush success, True otherwise.
 *                      If True, an exception with error description will be thrown.
 */
typeset CustomFlushCallback
{
    function void ();
    function void (any data);
    function bool (any data, char error[256]);
}


/**
 * Callback function called after the underlying CallbackSink object is destroyed.
 *
 * @note Can be used to close Handles in data.
 * @note The CallbackSink Handle is closed.
 *
 * @param data          Data value passed to the CallbackSink callbacks.
 */
typeset CustomDestoryPostCallback
{
    function void ();
    function void (any data);
}


/**
 * Trivial callback sink, gets a callback function and calls it on each log or flush.
 */
methodmap CallbackSink < Sink
{
    /**
     * Trivial callback sink, gets a callback function and calls it on each log or flush.
     *
     * @param logCB         Callback function called when Sink.Log() is called.
     * @param logPostCB     Callback function called after Sink.Log() is called.
     * @param flushCB       Callback function called when Sink.Flush() is called.
     * @param destoryPostCB Callback function called after the underlying CallbackSink object is destroyed.
     * @param data          Data value passed to the CallbackSin callbacks.
     * @return              CallbackSink handle.
     */
    public native CallbackSink(
        CustomLogCallback         logCB         = INVALID_FUNCTION,
        CustomLogPostCallback     logPostCB     = INVALID_FUNCTION,
        CustomFlushCallback       flushCB       = INVALID_FUNCTION,
        CustomDestoryPostCallback destoryPostCB = INVALID_FUNCTION,
        any data = 0);

    /**
     * Set data value passed to the CallbackSink callbacks.
     *
     * @param data              Data value.
     */
    public native void SetData(any data);

    /**
     * Get data value passed to the CallbackSink callbacks.
     */
    public native any GetData();

    /**
     * Create a logger handle that gets a callback function and calls it on each log or flush.
     *
     * @param name          The name of the new logger.
     * @param logCB         Callback function called when Sink.Log() is called.
     * @param logPostCB     Callback function called after Sink.Log() is called.
     * @param flushCB       Callback function called when Sink.Flush() is called.
     * @param destoryPostCB Callback function called after the underlying CallbackSink object is destroyed.
     * @param data          Data value passed to the CallbackSink callbacks.
     * @return              Logger handle.
     * @error               Logger name already exits.
     */
    public static native Logger CreateLogger(
        const char[] name,
        CustomLogCallback         logCB         = INVALID_FUNCTION,
        CustomLogPostCallback     logPostCB     = INVALID_FUNCTION,
        CustomFlushCallback       flushCB       = INVALID_FUNCTION,
        CustomDestoryPostCallback destoryPostCB = INVALID_FUNCTION,
        any data = 0);
}

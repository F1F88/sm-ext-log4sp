#if defined _log4sp_sinks_callback_sink2_included
 #endinput
#endif
#define _log4sp_sinks_callback_sink2_included

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
 * @param data          Data value passed to the CallbackSink2 callbacks.
 * @param error         Error message buffer in case of log failure.
 * @return              False for log success, True otherwise.
 *                      If True, an exception with error description will be thrown.
 */
typeset CustomLog2Callback
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
 * @param error         Error message buffer in case of log failure.
 * @param msg           Log messages that have been formatted in a pre-set pattern.
 * @param data          Data value passed to the CallbackSink2 callbacks.
 * @return              False for log success, True otherwise.
 *                      If True, an exception with error description will be thrown.
 */
typeset CustomLogPost2Callback
{
    function void (const char[] msg);
    function void (const char[] msg, any data);
    function bool (const char[] msg, any data, char error[256]);
}


/**
 * Callback function called when Sink.Flush() is called.
 *
 * @param error         Error message buffer in case of flush failure.
 * @param data          Data value passed to the CallbackSink2 callbacks.
 * @return              False for flush success, True otherwise.
 *                      If True, an exception with error description will be thrown.
 */
typeset CustomFlush2Callback
{
    function void ();
    function void (any data);
    function bool (any data, char error[256]);
}


/**
 * Callback function called after the underlying CallbackSink2 object is destroyed.
 *
 * @note Can be used to close Handles in data.
 * @note The CallbackSink2 Handle is closed.
 *
 * @param data          Data value passed to the CallbackSink2 callbacks.
 */
typeset CustomDestoryPost2Callback
{
    function void (any data);
}


/**
 * Trivial callback sink, gets a callback function and calls it on each log or flush.
 */
methodmap CallbackSink2 < Sink
{
    /**
     * Trivial callback sink, gets a callback function and calls it on each log or flush.
     *
     * @param logCB         Callback function called when Sink.Log() is called.
     * @param logPostCB     Callback function called after Sink.Log() is called.
     * @param flushCB       Callback function called when Sink.Flush() is called.
     * @param destoryPostCB Callback function called after the underlying CallbackSink2 object is destroyed.
     * @param data          Data value passed to the CallbackSink2 callbacks.
     * @return              CallbackSink2 handle.
     */
    public native CallbackSink2(
        CustomLog2Callback         logCB         = INVALID_FUNCTION,
        CustomLogPost2Callback     logPostCB     = INVALID_FUNCTION,
        CustomFlush2Callback       flushCB       = INVALID_FUNCTION,
        CustomDestoryPost2Callback destoryPostCB = INVALID_FUNCTION,
        any data = 0);

    /**
     * Set data value passed to the CallbackSink2 callbacks.
     *
     * @param data              Data value.
     */
    public native void SetData(any data);

    /**
     * Get data value passed to the CallbackSink2 callbacks.
     */
    public native any GetData();

    /**
     * Create a logger handle that gets a callback function and calls it on each log or flush.
     *
     * @param name          The name of the new logger.
     * @param logCB         Callback function called when Sink.Log() is called.
     * @param logPostCB     Callback function called after Sink.Log() is called.
     * @param flushCB       Callback function called when Sink.Flush() is called.
     * @param destoryPostCB Callback function called after the underlying CallbackSink2 object is destroyed.
     * @param data          Data value passed to the CallbackSink2 callbacks.
     * @return              Logger handle.
     * @error               Logger name already exits.
     */
    public static native Logger CreateLogger(
        const char[] name,
        CustomLog2Callback         logCB         = INVALID_FUNCTION,
        CustomLogPost2Callback     logPostCB     = INVALID_FUNCTION,
        CustomFlush2Callback       flushCB       = INVALID_FUNCTION,
        CustomDestoryPost2Callback destoryPostCB = INVALID_FUNCTION,
        any data = 0);
}

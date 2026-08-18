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
 * @param sink          CallbackSink Handle.
 *                      This is cloned from an existing handle.
 *                      It will automatically close after the callback function finishes.
 * @param error         Error message buffer in case of log failure.
 * @param maxlen        Maximum length of the error.
 * @param logTime       Log message timestamps (in nanoseconds).
 * @param loc           Source location.
 * @param name          The name of the logger.
 * @param lvl           Log level enum.
 * @param msg           Pre formatted log message.
 * @return              True on success, false on error.
 */
typedef CallbackSinkLogFunction = function bool (
    Sink sink,
    char[] error,
    int maxlen,
    const char[] logTime,
    const SourceLoc loc,
    const char[] name,
    LogLevel lvl,
    const char[] msg
);

/**
 * Callback function called when Sink.Flush() is called.
 *
 * @param sink          CallbackSink Handle.
 *                      This is cloned from an existing handle.
 *                      It will automatically close after the callback function finishes.
 * @param error         Error message buffer in case of flush failure.
 * @param maxlen        Maximum length of the error.
 * @return              True on success, false on error.
 */
typedef CallbackSinkFlushFunction = function bool (Sink sink, char[] error, int maxlen);

/**
 * Callback function called when the CallbackSink Handle is closed.
 *
 * @param data          Data value in CallbackSink.
 */
typedef CallbackSinkCloseFunction = function void (any data);


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
     * @param logPlugin     Handle of the plugin that contains the log function.
     *                      If null, defaults to the calling plugin.
     * @param logFunc       Function to call when CallbackSink.Log() is called.
     * @param flushPlugin   Handle of the plugin that contains the flush function.
     *                      If null, defaults to the calling plugin.
     * @param flushFunc     Function to call when CallbackSink.Flush() is called.
     * @param closePlugin   Handle of the plugin that contains the close function.
     *                      If null, defaults to the calling plugin.
     * @param closeFunc     Function to call after CallbackSink Handle is closed.
     * @param data          Sets a value in CallbackSink.
     * @return              A new CallbackSink Handle.
     */
    public native CallbackSink(Handle logPlugin, CallbackSinkLogFunction logFunc,
                               Handle flushPlugin, CallbackSinkFlushFunction flushFunc,
                               Handle closePlugin, CallbackSinkCloseFunction closeFunc,
                               any data=0);

    /**
     * Retrieves a value from CallbackSink.
     *
     * @return              Data value.
     */
    public native any GetData();

    /**
     * Sets a value in CallbackSink.
     *
     * @param data          Data value.
     */
    public native void SetData(any data);

    /**
     * Returns whether or not a callback sink handle is valid.
     *
     * @return          True if valid, false otherwise.
     */
    public native bool IsValid();
}

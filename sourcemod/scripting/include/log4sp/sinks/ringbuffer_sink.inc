#if defined _log_for_sourcepawn_sinks_ringbuffer_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_ringbuffer_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>


/**
 * Callback function called on DrainCallback.Drain().
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
typeset DrainCallback
{
    function void (const char[] name, LogLevel lvl, const char[] msg);
    function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func);
    function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int seconds[2]);
    function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int seconds[2], int nanoseconds[2]);
    function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int seconds[2], int nanoseconds[2], any data);
}


/**
 * Callback function called on DrainCallback.DrainFormatted().
 *
 * @param msg           Log messages that have been formatted in a pre-set pattern.
 * @param data          Data value passed to Logger.ApplyAll().
 */
typeset DrainFormattedCallback
{
    function void (const char[] msg);
    function void (const char[] msg, any data);
}


/**
 * Ring buffer sink. Holds fixed amount of log messages in memory.
 * When the buffer is full, new messages override the old ones.
 * Useful for storing debug data in memory in case of error.
 */
methodmap RingBufferSink < Sink
{
    /**
     * Ring buffer sink. Holds fixed amount of log messages in memory.
     * When the buffer is full, new messages override the old ones.
     * Useful for storing debug data in memory in case of error.
     *
     * @param           Amount of log messages in memory.
     * @return          RingBufferSink handle.
     */
    public native RingBufferSink(int amount);

    /**
     * Apply a user defined function on all buffered log messages.
     *
     * @param callback  A callback that apply a user defined function on all buffered log messages.
     * @param data      Extra data value to pass to the callback.
     * @error           Invalid drain callback.
     */
    public native void Drain(DrainCallback callback, any data = 0);

    /**
     * Apply a user defined function on all buffered log messages.
     *
     * @param callback  A callback that apply a user defined function on all buffered log messages.
     * @param data      Extra data value to pass to the callback.
     * @error           Invalid drain formatted callback.
     */
    public native void DrainFormatted(DrainFormattedCallback callback, any data = 0);

    /**
     * Create a logger handle that holds fixed amount of log messages in memory.
     *
     * @param name      The name of the new logger.
     * @param           Amount of log messages in memory.
     * @return          Logger handle.
     * @error           Logger name already exits.
     */
    public static native Logger CreateLogger(const char[] name, int amount);
}

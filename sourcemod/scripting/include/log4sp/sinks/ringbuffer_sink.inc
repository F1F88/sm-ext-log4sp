#if defined _log4sp_sinks_ringbuffer_sink_included
    #endinput
#endif
#define _log4sp_sinks_ringbuffer_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>


/**
 * Callback function called on RingBufferSink.DrainLatest() or RingBufferSink.DrainOldest().
 *
 * @param logTime       Log message timestamps (in nanoseconds).
 * @param loc           Source location.
 * @param name          The name of the logger.
 * @param lvl           Log level enum.
 * @param msg           Pre formatted log message.
 * @param data          Data value passed to callback.
 */
typeset DrainFunction
{
    function void (const char[] logTime, const SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg);
    function void (const char[] logTime, const SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg, any data);
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
     * @note RingBufferSink handles must be freed via delete or CloseHandle().
     *
     * @param maxSize   Maximum number of log messages.
     * @return          A new RingBufferSink Handle.
     */
    public native RingBufferSink(int maxSize);

    /**
     * Apply a user defined function on the latest buffered log messages.
     *
     * @param plugin    Handle of the plugin that contains the function.
     *                  If null, defaults to the calling plugin.
     * @param func      Function to call.
     * @param data      Extra data value to pass to the callback.
     * @return          True if can be drain log message, false otherwise.
     * @error           Invalid or corrupt plugin handle, or invalid function.
     */
    public native bool DrainLatest(Handle plugin, DrainFunction func, any data=0);

    /**
     * Apply a user defined function on the oldest buffered log messages.
     *
     * @param plugin    Handle of the plugin that contains the function.
     *                  If null, defaults to the calling plugin.
     * @param func      Function to call.
     * @param data      Extra data value to pass to the callback.
     * @return          True if can be drain log message, false otherwise.
     * @error           Invalid or corrupt plugin handle, or invalid function.
     */
    public native bool DrainOldest(Handle plugin, DrainFunction func, any data=0);

    /**
     * Get the maximum number of log messages that can be stored in the buffer.
     *
     * @return          Maximum number of log messages.
     */
    public native int GetMaxSize();

    /**
     * Get the actual number of stored log messages.
     *
     * @return          The actual number of log messages stored.
     */
    public native int GetSize();

    /**
     * Returns whether or not a ring buffer sink handle is valid.
     * Validates the base Sink properties and the ring buffer specific internal handles.
     *
     * @return          True if valid, false otherwise.
     */
    public native bool IsValid();
}

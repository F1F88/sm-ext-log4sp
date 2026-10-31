#if defined _log4sp_sinks_ringbuffer_sink_included
    #endinput
#endif
#define _log4sp_sinks_ringbuffer_sink_included

#include <log4sp/sinks/sink>

#if defined LOG4SP_HEADER_ONLY
#include <log4sp/details/circular_deque>
#endif      // LOG4SP_HEADER_ONLY


/**
 * Callback function called on RingBufferSink.DrainLatest() or RingBufferSink.DrainOldest().
 *
 * @param logTime       Log message timestamps (in milliseconds).
 * @param loc           Source location.
 * @param name          The name of the logger.
 * @param lvl           Log level enum.
 * @param msg           Pre formatted log message.
 * @param data          Data value passed to callback.
 */
typeset RingBufferDrainFunction
{
    function void (LOG4SP_CONST_ES Long logTime, LOG4SP_CONST_ES SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg);
    function void (LOG4SP_CONST_ES Long logTime, LOG4SP_CONST_ES SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg, any data);
}


/**
 * Ring buffer sink. Holds fixed amount of log messages in memory.
 * When the buffer is full, new messages override the old ones.
 * Useful for storing debug data in memory in case of error.
 */
methodmap RingBufferSink < Sink
{
#if !defined LOG4SP_HEADER_ONLY
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
    public native bool DrainLatest(Handle plugin, RingBufferDrainFunction func, any data=0);

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
    public native bool DrainOldest(Handle plugin, RingBufferDrainFunction func, any data=0);

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

#else

    /**
     * Ring buffer sink. Holds fixed amount of log messages in memory.
     * When the buffer is full, new messages override the old ones.
     * Useful for storing debug data in memory in case of error.
     *
     * @note RingBufferSink handles must be freed via Sink.Close().
     * @note Multi-plugin shared Handles must be freed before unload the plugin.
     *
     * @param maxSize   Maximum number of log messages.
     * @return          A new RingBufferSink Handle.
     */
    public RingBufferSink(int maxSize) {
        RingBufferSink sink = view_as<RingBufferSink>(Sink.__Sink("RingBufferSink",
                                                                  null, __RingBufferSink_OnLog,
                                                                  null, INVALID_FUNCTION,
                                                                  null, __RingBufferSink_OnClose));
        sink.__SetMaxSize(maxSize);
        sink.__SetLogTimes(new CircularDeque(maxSize, sizeof(Long)));
        sink.__SetLocs(new CircularDeque(maxSize, sizeof(SourceLoc)));
        sink.__SetNames(new CircularDeque(maxSize, ByteCountToCells(128))); // HACK: Hard-coded size
        sink.__SetLvls(new CircularDeque(maxSize));
        sink.__SetMsgs(new CircularDeque(maxSize, ByteCountToCells(LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH)));
        return sink;
    }

    public bool DrainLatest(Handle plugin, RingBufferDrainFunction func, any data=0) {
        if (!this.IsValid())
            ThrowError("Invalid RingBufferSink Handle %x.", this);

        CircularDeque logTimes = this.__GetLogTimes();
        if (logTimes.IsEmpty())
            return false;

        CircularDeque locs = this.__GetLocs();
        CircularDeque names = this.__GetNames();
        CircularDeque lvls = this.__GetLvls();
        CircularDeque msgs = this.__GetMsgs();

        Long logTime;
        SourceLoc loc;
        char name[128];
        char msg[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];

        logTimes.GetBackArray(logTime);
        locs.GetBackArray(loc);
        names.GetBackString(name, sizeof(name));
        LogLevel lvl = lvls.GetBack();
        msgs.GetBackString(msg, sizeof(msg));

        logTimes.PopBack();
        locs.PopBack();
        names.PopBack();
        lvls.PopBack();
        msgs.PopBack();

        // void (LOG4SP_CONST_ES Long logTime, LOG4SP_CONST_ES SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg, any data);
        Call_StartFunction(plugin, func);
        Call_PushArray(logTime, sizeof(logTime));
        Call_PushArray(loc, sizeof(loc));
        Call_PushString(name);
        Call_PushCell(lvl);
        Call_PushString(msg);
        Call_PushCell(data);
        Call_Finish();
        return true;
    }

    public bool DrainOldest(Handle plugin, RingBufferDrainFunction func, any data=0) {
        if (!this.IsValid())
            ThrowError("Invalid RingBufferSink Handle %x.", this);

        CircularDeque logTimes = this.__GetLogTimes();
        if (logTimes.IsEmpty())
            return false;

        CircularDeque locs = this.__GetLocs();
        CircularDeque names = this.__GetNames();
        CircularDeque lvls = this.__GetLvls();
        CircularDeque msgs = this.__GetMsgs();

        Long logTime;
        SourceLoc loc;
        char name[128];
        char msg[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];

        logTimes.GetFrontArray(logTime);
        locs.GetFrontArray(loc);
        names.GetFrontString(name, sizeof(name));
        LogLevel lvl = lvls.GetFront();
        msgs.GetFrontString(msg, sizeof(msg));

        logTimes.PopFront();
        locs.PopFront();
        names.PopFront();
        lvls.PopFront();
        msgs.PopFront();

        // void (LOG4SP_CONST_ES Long logTime, LOG4SP_CONST_ES SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg, any data);
        Call_StartFunction(plugin, func);
        Call_PushArray(logTime, sizeof(logTime));
        Call_PushArray(loc, sizeof(loc));
        Call_PushString(name);
        Call_PushCell(lvl);
        Call_PushString(msg);
        Call_PushCell(data);
        Call_Finish();
        return true;
    }

    public int GetMaxSize() {
        if (!this.IsValid())
            ThrowError("Invalid RingBufferSink Handle %x.", this);
        return this.__GetMaxSize();
    }

    public int GetSize() {
        if (!this.IsValid())
            ThrowError("Invalid RingBufferSink Handle %x.", this);
        return this.__GetLogTimes().Length;
    }

    public bool IsValid() {
        if (!view_as<Sink>(this).IsValid())
            return false;

#if !defined LOG4SP_HEADER_ONLY_NO_VALID_HANDLE
        if (!this.__Contains("m_uMaxSize") || !this.__Contains("m_hLogTimes")
         || !this.__Contains("m_hLocs")    || !this.__Contains("m_hNames")
         || !this.__Contains("m_hLvls")    || !this.__Contains("m_hMsgs"))
            return false;
#endif      // LOG4SP_HEADER_ONLY_NO_VALID_HANDLE

        char classname[16];
        this.__GetClassname(classname, sizeof(classname));
        return StrEqual(classname, "RingBufferSink");
    }


    // *** Do not use the function below this line! *** //
    public int __GetMaxSize()                       { return this.__GetValue("m_uMaxSize"); }
    public void __SetMaxSize(int value)             { this.__SetValue("m_uMaxSize", value); }

    // Ownership and lifecycle are the same as this, closed in OnClose.
    public CircularDeque __GetLogTimes()            { return this.__GetValue("m_hLogTimes"); }
    public void __SetLogTimes(CircularDeque value)  { this.__SetValue("m_hLogTimes", value); }

    // Ownership and lifecycle are the same as this, closed in OnClose.
    public CircularDeque __GetLocs()                { return this.__GetValue("m_hLocs"); }
    public void __SetLocs(CircularDeque value)      { this.__SetValue("m_hLocs", value); }

    // Ownership and lifecycle are the same as this, closed in OnClose.
    public CircularDeque __GetNames()               { return this.__GetValue("m_hNames"); }
    public void __SetNames(CircularDeque value)     { this.__SetValue("m_hNames", value); }

    // Ownership and lifecycle are the same as this, closed in OnClose.
    public CircularDeque __GetLvls()                { return this.__GetValue("m_hLvls"); }
    public void __SetLvls(CircularDeque value)      { this.__SetValue("m_hLvls", value); }

    // Ownership and lifecycle are the same as this, closed in OnClose.
    public CircularDeque __GetMsgs()                { return this.__GetValue("m_hMsgs"); }
    public void __SetMsgs(CircularDeque value)      { this.__SetValue("m_hMsgs", value); }
#endif      // LOG4SP_HEADER_ONLY
}


#if defined LOG4SP_HEADER_ONLY
public bool __RingBufferSink_OnLog(RingBufferSink sink, char[] error, int maxlen, LOG4SP_CONST_ES Long logTime, LOG4SP_CONST_ES SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg)
{
    sink.__GetLogTimes().PushBackArray(logTime);
    sink.__GetLocs().PushBackArray(loc);
    sink.__GetNames().PushBackString(name);
    sink.__GetLvls().PushBack(lvl);
    sink.__GetMsgs().PushBackString(msg);
    return true;
}

public void __RingBufferSink_OnClose(RingBufferSink sink)
{
    sink.__GetLogTimes().Close();
    sink.__GetLocs().Close();
    sink.__GetNames().Close();
    sink.__GetLvls().Close();
    sink.__GetMsgs().Close();

    // It will be destroyed soon, so ignore resetting it to null.
    // sink.__SetLogTimes(null);
    // sink.__SetLocs(null);
    // sink.__SetNames(null);
    // sink.__SetLvls(null);
    // sink.__SetMsgs(null);
}
#endif      // LOG4SP_HEADER_ONLY

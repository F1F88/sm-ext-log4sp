#if defined _log4sp_sinks_ringbuffer_sink_included
    #endinput
#endif
#define _log4sp_sinks_ringbuffer_sink_included

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
    function void (const char[] logTime, LOG4SP_ES_CONST SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg);
    function void (const char[] logTime, LOG4SP_ES_CONST SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg, any data);
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
        sink.__SetLogTimes(new ArrayList(ByteCountToCells(22)));
        sink.__SetLocs(new ArrayList(sizeof(SourceLoc)));
        sink.__SetNames(new ArrayList(ByteCountToCells(128)));
        sink.__SetLvls(new ArrayList(_));
        sink.__SetMsgs(new ArrayList(ByteCountToCells(LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH)));
        return sink;
    }

    public bool DrainLatest(Handle plugin, DrainFunction func, any data=0) {
        if (!this.IsValid())
            ThrowError("Invalid RingBufferSink Handle %x.", this);

        ArrayList logTimes = this.__GetLogTimes();
        int length = logTimes.Length;
        if (length <= 0)
            return false;

        ArrayList locs = this.__GetLocs();
        ArrayList names = this.__GetNames();
        ArrayList lvls = this.__GetLvls();
        ArrayList msgs = this.__GetMsgs();

        int index = length - 1;
        SourceLoc loc;
        char logTime[21], name[128], msg[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];

        logTimes.GetString(index, logTime, sizeof(logTime));
        locs.GetArray(index, loc);
        names.GetString(index, name, sizeof(name));
        LogLevel lvl = lvls.Get(index);
        msgs.GetString(index, msg, sizeof(msg));

        logTimes.Erase(index);
        locs.Erase(index);
        names.Erase(index);
        lvls.Erase(index);
        msgs.Erase(index);

        // void (const char[] logTime, LOG4SP_ES_CONST SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg, any data);
        Call_StartFunction(plugin, func);
        Call_PushString(logTime);
        Call_PushArray(loc, sizeof(loc));
        Call_PushString(name);
        Call_PushCell(lvl);
        Call_PushString(msg);
        Call_PushCell(data);
        Call_Finish();
        return true;
    }

    public bool DrainOldest(Handle plugin, DrainFunction func, any data=0) {
        if (!this.IsValid())
            ThrowError("Invalid RingBufferSink Handle %x.", this);

        ArrayList logTimes = this.__GetLogTimes();
        int length = logTimes.Length;
        if (length <= 0)
            return false;

        ArrayList locs = this.__GetLocs();
        ArrayList names = this.__GetNames();
        ArrayList lvls = this.__GetLvls();
        ArrayList msgs = this.__GetMsgs();

        int index = 0;
        SourceLoc loc;
        char logTime[21], name[128], msg[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];

        logTimes.GetString(index, logTime, sizeof(logTime));
        locs.GetArray(index, loc);
        names.GetString(index, name, sizeof(name));
        LogLevel lvl = lvls.Get(index);
        msgs.GetString(index, msg, sizeof(msg));

        logTimes.Erase(index);
        locs.Erase(index);
        names.Erase(index);
        lvls.Erase(index);
        msgs.Erase(index);

        // void (const char[] logTime, LOG4SP_ES_CONST SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg, any data);
        Call_StartFunction(plugin, func);
        Call_PushString(logTime);
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
    public void __SetMaxSize(int maxSize)           { this.__SetValue("m_uMaxSize", maxSize); }

    // Ownership and lifecycle are the same as this, closed in OnClose.
    public ArrayList __GetLogTimes()                { return this.__GetValue("m_hLogTimes"); }
    public void __SetLogTimes(ArrayList logTimes)   { this.__SetValue("m_hLogTimes", logTimes); }

    // Ownership and lifecycle are the same as this, closed in OnClose.
    public ArrayList __GetLocs()                    { return this.__GetValue("m_hLocs"); }
    public void __SetLocs(ArrayList locs)           { this.__SetValue("m_hLocs", locs); }

    // Ownership and lifecycle are the same as this, closed in OnClose.
    public ArrayList __GetNames()                   { return this.__GetValue("m_hNames"); }
    public void __SetNames(ArrayList names)         { this.__SetValue("m_hNames", names); }

    // Ownership and lifecycle are the same as this, closed in OnClose.
    public ArrayList __GetLvls()                    { return this.__GetValue("m_hLvls"); }
    public void __SetLvls(ArrayList lvls)           { this.__SetValue("m_hLvls", lvls); }

    // Ownership and lifecycle are the same as this, closed in OnClose.
    public ArrayList __GetMsgs()                    { return this.__GetValue("m_hMsgs"); }
    public void __SetMsgs(ArrayList msgs)           { this.__SetValue("m_hMsgs", msgs); }
#endif      // LOG4SP_HEADER_ONLY
}


#if defined LOG4SP_HEADER_ONLY
public bool __RingBufferSink_OnLog(RingBufferSink sink, char[] error, int maxlen, const char[] logTime, LOG4SP_ES_CONST SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg)
{
    int maxSize = sink.__GetMaxSize();
    if (maxSize)
    {
        ArrayList logTimes = sink.__GetLogTimes();
        ArrayList locs = sink.__GetLocs();
        ArrayList names = sink.__GetNames();
        ArrayList lvls = sink.__GetLvls();
        ArrayList msgs = sink.__GetMsgs();

        if (logTimes.Length >= maxSize)
        {
            logTimes.Erase(0);
            locs.Erase(0);
            names.Erase(0);
            lvls.Erase(0);
            msgs.Erase(0);
        }

        logTimes.PushString(logTime);
        locs.PushArray(loc, sizeof(loc));
        names.PushString(name);
        lvls.Push(lvl);
        msgs.PushString(msg);
    }
    return true;
}

public void __RingBufferSink_OnClose(RingBufferSink sink)
{
    delete sink.__GetLogTimes();
    delete sink.__GetLocs();
    delete sink.__GetNames();
    delete sink.__GetLvls();
    delete sink.__GetMsgs();

    // It will be destroyed soon, so ignore resetting it to null.
    // sink.__SetLogTimes(null);
    // sink.__SetLocs(null);
    // sink.__SetNames(null);
    // sink.__SetLvls(null);
    // sink.__SetMsgs(null);
}
#endif      // LOG4SP_HEADER_ONLY

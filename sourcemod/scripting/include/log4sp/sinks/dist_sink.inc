#if defined _log4sp_sinks_dist_sink_included
    #endinput
#endif
#define _log4sp_sinks_dist_sink_included

#include <log4sp/sinks/sink>


/**
 * Distribution sink.
 * Stores a list of sinks which get called when log is called.
 */
methodmap DistSink < Sink
{
#if !defined LOG4SP_HEADER_ONLY
    /**
     * Stores a list of sinks which get called when log is called.
     *
     * @note DistSink handles must be freed via delete or CloseHandle().
     *
     * @return          A new DistSink Handle.
     */
    public native DistSink();

    /**
     * Gets the sub-sink clone of sink.
     *
     * @note Cloned sub-sink handle must be freed via delete or CloseHandle() or Sink.Close().
     *
     * @param index     Index in the sinks (starting from 0).
     * @return          Cloned sub-sink handle.
	 * @error           Invalid index.
     */
    public native Sink GetSink(int index);

    /**
     * Gets the length of the sub-sinks.
     *
     * @return          Length of the sub-sinks.
     */
    public native int GetSinksLength();

    /**
     * Clone the sub-sink handle and add it to the sub-sinks.
     *
     * @param subSink   Sub-sink Handle.
     * @return          True on success, false if it already exist.
     * @error           Invalid sub-sink handle.
     */
    public native bool AddSink(Sink subSink);

    /**
     * Clone the sub-sink handle and add it to the sub-sinks.
     *
     * @param subSink   Sub-sink Handle.
     * @return          True on success, false if it already exist.
     * @error           Invalid sub-sink handle.
     */
    public bool AddSinkAndClose(Sink subSink) {
        bool added = this.AddSink(subSink);
        subSink.Close();
        return added;
    }

    /**
     * Close the cloned sub-sink handle and remove from sub-sinks.
     *
     * @param subSink   Sub-sink Handle.
     * @return          True on success, false if it not exist.
     * @error           Invalid sub-sink handle.
     */
    public native bool DropSink(Sink subSink);

    /**
     * Returns whether or not a dist sink handle is valid.
     *
     * @return          True if valid, false otherwise.
     */
    public native bool IsValid();

#else

    /**
     * Stores a list of sinks which get called when log is called.
     *
     * @note DistSink handles must be freed via Sink.Close().
     * @note Multi-plugin shared Handles must be freed before unload the plugin.
     *
     * @return          A new DistSink Handle.
     */
    public DistSink() {
        ArrayList sinks = new ArrayList();
        ArrayList owners = new ArrayList(.blocksize=PLATFORM_MAX_PATH);

        DistSink sink = view_as<DistSink>(Sink.__Sink("DistSink",
                                                      null, __DistSink_OnLog,
                                                      null, __DistSink_OnFlush,
                                                      null, __DistSink_OnClose));
        sink.__SetSinks(sinks);
        sink.__SetSinksOwner(owners);
        return sink;
    }

    /**
     * Gets the sub-sink clone of sink.
     *
     * @note Cloned sub-sink handle must be freed via Sink.Close().
     *
     * @param index     Index in the sinks (starting from 0).
     * @return          Cloned sub-sink handle.
	 * @error           Invalid index.
     */
    public Sink GetSink(int index) {
        if (!this.IsValid())
            ThrowError("Invalid DistSink Handle %x.", this);

        ArrayList sinks = this.__GetSinks();
        int length = sinks.Length;
        if (UInt32Compare(index, length) >= 0)
            ThrowError("Invalid index %u. [0-%u]", index, length);

        Sink sink = sinks.Get(index);
        return sink.Clone();
    }

    public int GetSinksLength() {
        if (!this.IsValid())
            ThrowError("Invalid DistSink Handle %x.", this);

        return this.__GetSinks().Length;
    }

    public bool AddSink(Sink subSink) {
        if (!this.IsValid())
            ThrowError("Invalid DistSink Handle %x.", this);
        if (!subSink.IsValid())
            ThrowError("Invalid Sink Handle %x.", subSink);

        if (this.Equals(subSink))
            return false;

        ArrayList sinks = this.__GetSinks();
        ArrayList owners = this.__GetSinksOwner();
        for (int i = 0; i < sinks.Length; ++i) {
            Sink cloned = sinks.Get(i);
            if (cloned.Equals(subSink))
                return false;
        }

        // Save the identity of the adder
        char owner[PLATFORM_MAX_PATH];
        GetPluginFilename(null, owner, sizeof(owner));

        // Cloning and then adding ensures that closing the original sink does
        // not affect the sink held by the dist sink.
        // Clones cannot be obtained externally;
        // they can only be closed using "distSink.Drop()".
        sinks.Push(subSink.Clone());
        owners.PushString(owner);
        return true;
    }

    public bool AddSinkAndClose(Sink subSink) {
        bool added = this.AddSink(subSink);
        subSink.Close();
        return added;
    }

    public bool DropSink(Sink subSink) {
        if (!this.IsValid())
            ThrowError("Invalid DistSink Handle %x.", this);
        if (!subSink.IsValid())
            ThrowError("Invalid Sink Handle %x.", subSink);

        // Remove and close
        ArrayList sinks = this.__GetSinks();
        ArrayList owners = this.__GetSinksOwner();
        for (int i = 0; i < sinks.Length; ++i) {
            Sink cloned = sinks.Get(i);
            if (!cloned.Equals(subSink))
                continue;

            // Close as the adder
            char owner[PLATFORM_MAX_PATH];
            owners.GetString(i, owner, sizeof(owner));
            Handle plugin = FindPluginByFile(owner);
            Function func = GetFunctionByName(plugin, "__DistSink_OnDropSink");

            // void (Sink sink);
            Call_StartFunction(plugin, func);
            Call_PushCell(cloned);
            Call_Finish();

            // Remove
            sinks.Erase(i);
            owners.Erase(i);
            return true;
        }
        return false;
    }

    public bool IsValid() {
        if (!view_as<Sink>(this).IsValid())
            return false;

#if !defined LOG4SP_HEADER_ONLY_NO_VALID_HANDLE
        if (!this.__Contains("m_hSinks") || !this.__Contains("m_hSinksOwner"))
            return false;
#endif      // LOG4SP_HEADER_ONLY_NO_VALID_HANDLE

        char classname[16];
        this.__GetClassname(classname, sizeof(classname));
        return StrEqual(classname, "DistSink") || StrEqual(classname, "DupFilterSink");
    }


    // *** Do not use the function below this line! *** //
    /**
     * Ownership and lifecycle are the same as this, closed in __DistSink_OnClose().
     * Ownership and lifecycle of elements are uncertain,
     * they are closed individually based on SinksOwner.
     */
    public ArrayList __GetSinks()                       { return this.__GetValue("m_hSinks"); }
    public void __SetSinks(ArrayList value)             { this.__SetValue("m_hSinks", value); }

    /**
     * After plugin A calls "new Sink() & distSink.AddSink(sink)",
     * ownership of the sink handle cloned by distSink belongs to Plugin A.
     *
     * If Plugin B calls "distSink.Close()" and refCnt <= 0,
     * the sink handle cannot be close using plugin B identity.
     *
     * Sink handle need to be close as plugin A.
     *
     * SinksOwner is used to store and retrieve Plugin A.
     * Ultimately, calling Plugin A's __DistSink_OnClose() cleanup the sink handle.
     *
     * Ownership and lifecycle are the same as this, closed in __DistSink_OnClose().
     */
    public ArrayList __GetSinksOwner()                  { return this.__GetValue("m_hSinksOwner"); }
    public void __SetSinksOwner(ArrayList value)        { this.__SetValue("m_hSinksOwner", value); }
#endif      // LOG4SP_HEADER_ONLY
}


#if defined LOG4SP_HEADER_ONLY
public void __DistSink_OnDropSink(Sink sink)
{
    sink.Close();
}

public bool __DistSink_OnLog(DistSink sink, char[] error, int maxlen, LOG4SP_CONST_ES Long logTime, LOG4SP_CONST_ES SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg)
{
    ArrayList sinks = sink.__GetSinks();
    int len = sinks.Length;

    for (int i = 0; i < len; ++i)
    {
        Sink subSink = sinks.Get(i);
        if (!subSink.ShouldLog(lvl))
            continue;

        if (!subSink.Log(logTime, loc, name, lvl, msg, error, maxlen))
            return false;
    }
    return true;
}

public bool __DistSink_OnFlush(DistSink sink, char[] error, int maxlen)
{
    ArrayList sinks = sink.__GetSinks();
    int len = sinks.Length;

    for (int i = 0; i < len; ++i)
    {
        Sink subSink = sinks.Get(i);
        if (!subSink.Flush(error, maxlen))
            return false;
    }
    return true;
}

public void __DistSink_OnClose(DistSink sink)
{
    ArrayList sinks = sink.__GetSinks();
    ArrayList owners = sink.__GetSinksOwner();

    for (int i = sinks.Length - 1; i >= 0; --i)
    {
        Sink subSink = sinks.Get(i);
        sink.DropSink(subSink);
    }

    delete sinks;
    delete owners;
}
#endif      // LOG4SP_HEADER_ONLY

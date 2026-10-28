#if defined _log4sp_sinks_dup_filter_sink_included
    #endinput
#endif
#define _log4sp_sinks_dup_filter_sink_included

#include <log4sp/sinks/dist_sink>
#include <log4sp/details/long>


/**
 * Duplicate message removal sink.
 * Skip the message if previous one is identical and less than "max skip duration" have passed.
 */
methodmap DupFilterSink < DistSink
{
#if !defined LOG4SP_HEADER_ONLY
    /**
     * Duplicate message removal sink.
     * Skip the message if previous one is identical and less than "max skip duration" have passed.
     *
     * @note DupFilterSink handles must be freed via delete or CloseHandle().
     *
     * @param duration      Maximum skip duration in milliseconds.
     *                      Messages identical to the previous one within this window are skipped.
     * @return              A new DupFilterSink Handle.
     */
    public native DupFilterSink(const Long duration);

    /**
     * Returns whether or not a duplicate filter sink handle is valid.
     *
     * @return          True if valid, false otherwise.
     */
    public native bool IsValid();

#else

    /**
     * Duplicate message removal sink.
     * Skip the message if previous one is identical and less than "maxSkipDuration" have passed.
     *
     * @note DupFilterSink handles must be freed via Sink.Close().
     * @note Multi-plugin shared Handles must be freed before unload the plugin.
     *
     * @param duration      Maximum skip duration in milliseconds.
     *                      Messages identical to the previous one within this window are skipped.
     * @return              A new DupFilterSink Handle.
     */
    public DupFilterSink(LOG4SP_CONST_ES Long duration) {
        ArrayList sinks = new ArrayList();
        ArrayList owners = new ArrayList(.blocksize=PLATFORM_MAX_PATH);

        DupFilterSink sink = view_as<DupFilterSink>(Sink.__Sink("DupFilterSink",
                                                                null, __DupFilterSink_OnLog,
                                                                null, __DupFilterSink_OnFlush,
                                                                null, __DupFilterSink_OnClose));
        sink.__SetSinks(sinks);
        sink.__SetSinksOwner(owners);
        sink.__SetMaxSkipDuration(duration);
        sink.__SetLastLogTime(IntToLong(0));
        sink.__SetLastMsg(NULL_STRING);
        sink.__SetSkipCounter(0);
        sink.__SetSkipLogLevel(LogLevel_Off);
        return sink;
    }

    public bool IsValid() {
        if (!view_as<DistSink>(this).IsValid())
            return false;

#if !defined LOG4SP_HEADER_ONLY_NO_VALID_HANDLE
        if (!this.__Contains("m_lMaxSkipDuration") || !this.__Contains("m_lLastLogTime")
         || !this.__Contains("m_szLastMsg")        || !this.__Contains("m_uSkipCounter")
         || !this.__Contains("m_eSkipLogLvl"))
            return false;
#endif      // LOG4SP_HEADER_ONLY_NO_VALID_HANDLE

        char classname[16];
        this.__GetClassname(classname, sizeof(classname));
        return StrEqual(classname, "DupFilterSink");
    }


    // *** Do not use the function below this line! *** //
    public Long __GetMaxSkipDuration()                  { return this.__GetLong("m_lMaxSkipDuration"); }
    public void __SetMaxSkipDuration(Long value)        { this.__SetLong("m_lMaxSkipDuration", value); }

    public Long __GetLastLogTime()                      { return this.__GetLong("m_lLastLogTime"); }
    public void __SetLastLogTime(Long value)            { this.__SetLong("m_lLastLogTime", value); }

    public int __GetLastMsg(char[] value, int maxlen)   { return this.__GetString("m_szLastMsg", value, maxlen); }
    public void __SetLastMsg(const char[] value)        { this.__SetString("m_szLastMsg", value); }

    public int __GetSkipCounter()                       { return this.__GetValue("m_uSkipCounter"); }
    public void __SetSkipCounter(int value)             { this.__SetValue("m_uSkipCounter", value); }

    public LogLevel __GetSkipLogLevel()                 { return this.__GetValue("m_eSkipLogLvl"); }
    public void __SetSkipLogLevel(LogLevel value)       { this.__SetValue("m_eSkipLogLvl", value); }

    public bool __Filter(LOG4SP_CONST_ES Long logTime, const char[] msg) {
        Long filterDuration;
        filterDuration = logTime.Sub(this.__GetLastLogTime());
        if (filterDuration.Gt(this.__GetMaxSkipDuration()))
            return true;

        char buffer[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];
        this.__GetLastMsg(buffer, sizeof(buffer));
        if (!StrEqual(buffer, msg))
            return true;
        return false;
    }
#endif      // LOG4SP_HEADER_ONLY
}


#if defined LOG4SP_HEADER_ONLY
public bool __DupFilterSink_OnLog(DupFilterSink sink, char[] error, int maxlen, LOG4SP_CONST_ES Long logTime, LOG4SP_CONST_ES SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg)
{
    int skipCounter = sink.__GetSkipCounter();
    bool filtered = sink.__Filter(logTime, msg);
    if (!filtered)
    {
        sink.__SetSkipCounter(skipCounter + 1);
        sink.__SetSkipLogLevel(lvl);
        return true;
    }

    // log the "skipped.." message
    if (skipCounter > 0)
    {
        char buffer[64];
        int bytes = FormatEx(buffer, sizeof(buffer), "Skipped %u duplicate messages.", skipCounter);

        if (bytes > 0 && bytes < sizeof(buffer))
        {
            if (!__DistSink_OnLog(view_as<DistSink>(sink), error, maxlen, logTime, loc, name, sink.__GetSkipLogLevel(), buffer))
                return false;
        }
    }

    // log current message
    if (!__DistSink_OnLog(sink, error, maxlen, logTime, loc, name, lvl, msg))
        return false;

    sink.__SetLastLogTime(logTime);
    sink.__SetSkipCounter(0);
    sink.__SetLastMsg(msg);
    return true;
}

public bool __DupFilterSink_OnFlush(DupFilterSink sink, char[] error, int maxlen)
{
    return __DistSink_OnFlush(view_as<DistSink>(sink), error, maxlen);
}

public void __DupFilterSink_OnClose(DupFilterSink sink)
{
    __DistSink_OnClose(view_as<DistSink>(sink));
}
#endif      // LOG4SP_HEADER_ONLY

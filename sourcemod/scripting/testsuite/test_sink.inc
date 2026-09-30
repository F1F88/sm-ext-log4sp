#if defined _log4sp_sinks_test_sink_included
 #endinput
#endif
#define _log4sp_sinks_test_sink_included

#include <log4sp/details/os>
#include <log4sp/sinks/callback_sink>


enum struct LogEvent
{
    char time[22];
    SourceLoc loc;
    char name[MAX_NAME_LENGTH];
    LogLevel lvl;
#if !defined LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH
    char msg[1024];
#else
    char msg[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];
#endif

    void Set(const char[] time, SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg) {
        strcopy(this.time, sizeof(LogEvent::time), time);
        strcopy(this.loc.filename, sizeof(SourceLoc::filename), loc.filename);
        this.loc.line = loc.line;
        strcopy(this.loc.funcname, sizeof(SourceLoc::funcname), loc.funcname);
        strcopy(this.name, sizeof(LogEvent::name), name);
        this.lvl = lvl;
        strcopy(this.msg, sizeof(LogEvent::msg), msg);
    }

    int ToString(char[] buffer, int maxlen) {
        char level[8];
        LogLevelToName(level, sizeof(level), this.lvl);
        return FormatEx(buffer, maxlen,
            "LogEvent{time=\"%s\",loc={func=\"%s\",line=%d,file=\"%s\"},name=\"%s\",lvl=\"%d\",msg=\"%s\"}.",
            this.time, this.loc.funcname, this.loc.line, this.loc.filename, this.name, level, this.msg);
    }

    void __Debug() {
        char buffer[1024];
        this.ToString(buffer, sizeof(buffer));
        PrintToServer("%s", buffer);
    }
}


methodmap TestSink < Sink
{
    public TestSink() {
        // The underlying implementation is actually a CallbackSink
        // Exposing the inheritance from Sink is intended to hide the CallbackSink.GetData().
        TestSinkData data = new TestSinkData();
        TestSink sink = view_as<TestSink>(new CallbackSink(null, __TestSink_OnLog,
                                                           null, __TestSink_OnFlush,
                                                           null, __TestSink_OnClose,
                                                           data));
        return sink;
    }

    public int GetLogCount() {
        return view_as<TestSinkData>(view_as<CallbackSink>(this).GetData()).logCount;
    }

    public void SetLogDelay(int milliseconds) {
        view_as<TestSinkData>(view_as<CallbackSink>(this).GetData()).logDelay = milliseconds;
    }

    public void SetLogError(const char[] logError) {
        view_as<TestSinkData>(view_as<CallbackSink>(this).GetData()).SetLogError(logError);
    }

    public void ClearLogError() {
        view_as<TestSinkData>(view_as<CallbackSink>(this).GetData()).SetLogError(NULL_STRING);
    }

    public ArrayList Drain() {
        ArrayList logEvents = view_as<TestSinkData>(view_as<CallbackSink>(this).GetData()).logEvents;
        if (logEvents.Length == 0)
            ThrowError("TestSink::logEvents is empty.");

        ArrayList copy = logEvents.Clone();
        logEvents.Clear();
        return copy;
    }

    public LogEvent DrainLatest() {
        ArrayList logEvents = view_as<TestSinkData>(view_as<CallbackSink>(this).GetData()).logEvents;
        if (logEvents.Length == 0)
            ThrowError("TestSink::logEvents is empty.");

        LogEvent logEvent;
        logEvents.GetArray(logEvents.Length - 1, logEvent);
        logEvents.Erase(logEvents.Length - 1);
        return logEvent;
    }

    public LogEvent DrainOldest() {
        ArrayList logEvents = view_as<TestSinkData>(view_as<CallbackSink>(this).GetData()).logEvents;
        if (logEvents.Length == 0)
            ThrowError("TestSink::logEvents is empty.");

        LogEvent logEvent;
        logEvents.GetArray(0, logEvent);
        logEvents.Erase(0);
        return logEvent;
    }

    public int GetFlushCount() {
        return view_as<TestSinkData>(view_as<CallbackSink>(this).GetData()).flushCount;
    }

    public void SetFlushDelay(int milliseconds) {
        view_as<TestSinkData>(view_as<CallbackSink>(this).GetData()).flushDelay = milliseconds;
    }

    public void SetFlushError(const char[] flushError) {
        view_as<TestSinkData>(view_as<CallbackSink>(this).GetData()).SetFlushError(flushError);
    }

    public void ClearFlushError() {
        view_as<TestSinkData>(view_as<CallbackSink>(this).GetData()).SetFlushError(NULL_STRING);
    }
}


public bool __TestSink_OnLog(TestSink sink, char[] error, int maxlen, const char[] logTime, LOG4SP_ES_CONST SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg)
{
    // Simulated delay
    SafeSleepForMilliseconds(view_as<TestSinkData>(view_as<CallbackSink>(sink).GetData()).logDelay);

    // Simulated error
    char logError[256];
    view_as<TestSinkData>(view_as<CallbackSink>(sink).GetData()).GetLogError(logError, sizeof(logError));
    if (logError[0])
    {
        strcopy(error, maxlen, logError);
        return false;
    }

    // Increase LogCounter
    view_as<TestSinkData>(view_as<CallbackSink>(sink).GetData()).logCount++;

    // Push LogEvent
    LogEvent logEvent;
    logEvent.Set(logTime, loc, name, lvl, msg);
    view_as<TestSinkData>(view_as<CallbackSink>(sink).GetData()).logEvents.PushArray(logEvent);
    return true;
}

public bool __TestSink_OnFlush(TestSink sink, char[] error, int maxlen)
{
    // Simulated delay
    SafeSleepForMilliseconds(view_as<TestSinkData>(view_as<CallbackSink>(sink).GetData()).flushDelay);

    // Simulated error
    char flushError[256];
    view_as<TestSinkData>(view_as<CallbackSink>(sink).GetData()).GetFlushError(flushError, sizeof(flushError));
    if (flushError[0])
    {
        strcopy(error, maxlen, flushError);
        return false;
    }

    // Increase FlushCounter
    view_as<TestSinkData>(view_as<CallbackSink>(sink).GetData()).flushCount++;
    return true;
}

public void __TestSink_OnClose(TestSinkData data)
{
    data.Close();
}



methodmap TestSinkData < Handle
{
    public TestSinkData() {
        char owner[PLATFORM_MAX_PATH];
        GetPluginFilename(null, owner, sizeof(owner));

        TestSinkData data = view_as<TestSinkData>(new StringMap());
        data.SetOwner(owner);
        data.logCount = 0;
        data.logDelay = 0;
        data.SetLogError(NULL_STRING);
        data.logEvents = new ArrayList(sizeof(LogEvent));
        data.flushCount = 0;
        data.flushDelay = 0;
        data.SetFlushError(NULL_STRING);
        return data;
    }

    public int GetOwner(char[] owner, int maxlen) {
        return this.__GetString("m_szOwner", owner, maxlen);
    }

    public void SetOwner(const char[] owner) {
        this.__SetString("m_szOwner", owner);
    }

    property int logCount {
        public get()                    { return this.__GetValue("m_cLogCount"); }
        public set(int logCount)        { this.__SetValue("m_cLogCount", logCount); }
    }

    property int logDelay {
        public get()                    { return this.__GetValue("m_uLogDelay"); }
        public set(int logDelay)        { this.__SetValue("m_uLogDelay", logDelay); }
    }

    public int GetLogError(char[] logError, int maxlen) {
        return this.__GetString("m_szLogError", logError, maxlen);
    }

    public void SetLogError(const char[] logError) {
        this.__SetString("m_szLogError", logError);
    }

    property ArrayList logEvents {
        public get()                    { return this.__GetValue("m_hLogEvents"); }
        public set(ArrayList logEvents) { this.__SetValue("m_hLogEvents", logEvents); }
    }

    property int flushCount {
        public get()                    { return this.__GetValue("m_cFlushCount"); }
        public set(int flushCount)      { this.__SetValue("m_cFlushCount", flushCount); }
    }

    property int flushDelay {
        public get()                    { return this.__GetValue("m_uFlushDelay"); }
        public set(int flushDelay)      { this.__SetValue("m_uFlushDelay", flushDelay);}
    }

    public int GetFlushError(char[] flushError, int maxlen) {
        return this.__GetString("m_szFlushError", flushError, maxlen);
    }

    public void SetFlushError(const char[] flushError) {
        this.__SetString("m_szFlushError", flushError);
    }

    public void Close() {
        char owner[PLATFORM_MAX_PATH];
        this.GetOwner(owner, sizeof(owner));
        Handle plugin = FindPluginByFile(owner);
        Function func = GetFunctionByName(plugin, "__TestSinkData_Close");

        // void (TestSinkData data);
        Call_StartFunction(plugin, func);
        Call_PushCell(this);
        Call_Finish();
        delete view_as<StringMap>(this);
    }

    public int ToString(char[] buffer, int maxlen) {
        char owner[PLATFORM_MAX_PATH];
        char logError[256];
        char flushError[256];
        this.GetOwner(owner, sizeof(owner));
        this.GetLogError(logError, sizeof(logError));
        this.GetFlushError(flushError, sizeof(flushError));

        return FormatEx(buffer, maxlen,
            "TestSinkData{owner=\"%s\",logCount=%d,logDelay=%d,logError=\"%s\",logEvents=%X,logEventsLength=%d,flushCount=%d,flushDelay=%d,flushError=\"%s\"}",
            owner, this.logCount, this.logDelay, logError, this.logEvents, this.logEvents.Length, this.flushCount, this.flushDelay, flushError);
    }

    public void __Debug() {
        char buffer[1024];
        this.ToString(buffer, sizeof(buffer));
        PrintToServer("%s", buffer);
        if (this.logEvents) {
            for (int i = 0; i < this.logEvents.Length; ++i) {
                LogEvent logEvent;
                this.logEvents.GetArray(i, logEvent, sizeof(logEvent));
                logEvent.ToString(buffer, sizeof(buffer));
                PrintToServer("[%d/%d] %s", i, this.logEvents.Length, buffer);
            }
        }
    }


    public bool __Contains(const char[] key) {
        return view_as<StringMap>(this).ContainsKey(key);
    }

    public any __GetValue(const char[] key) {
        any value;
        view_as<StringMap>(this).GetValue(key, value);
        return value;
    }

    public void __SetValue(const char[] key, any value) {
        view_as<StringMap>(this).SetValue(key, value);
    }

    public int __GetString(const char[] key, char[] value, int maxlen) {
        int size;
        view_as<StringMap>(this).GetString(key, value, maxlen, size);
        return size;
    }

    public void __SetString(const char[] key, const char[] value) {
        view_as<StringMap>(this).SetString(key, value);
    }
}


public void __TestSinkData_Close(TestSinkData data)
{
    delete data.logEvents;
}

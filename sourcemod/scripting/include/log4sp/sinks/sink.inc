#if defined _log4sp_sinks_sink_included
    #endinput
#endif
#define _log4sp_sinks_sink_included

#include <log4sp/common>


/**
 * Cleans up a sink and sets the passed variable to null.
 *
 * @param sink      Sink handle to clean up.
 */
stock void SinkCleanupAndDelete(Sink &sink)
{
    sink.Close();
    sink = null;
}

/**
 * Clean up the sinks array and set the elements of the array to null.
 *
 * @param sinks     Sink handles to clean up.
 * @param maxlen    Maximum length of the sinks.
 */
stock void SinksCleanupAndDelete(Sink[] sinks, int maxlen)
{
    for (int i = 0; i < maxlen; ++i)
    {
        sinks[i].Close();
        sinks[i] = null;
    }
}


/**
 * The Sink is the core component of the logging library responsible for the
 * actual output of log messages. Each Sink corresponds to a single log output
 * target, such as a file or the console.
 */
methodmap Sink < Handle
{
#if !defined LOG4SP_HEADER_ONLY
    /**
     * Logs a message.
     *
     * @param logTime   Log message timestamps (in nanoseconds).
     *                  If invalid, defaults to the current time.
     * @param loc       Source location.
     * @param name      The name of the logger.
     * @param lvl       Log level enum.
     * @param msg       Pre formatted log message.
     * @param error     Buffer to store error description.
     * @param maxlen    Maximum length of the error.
     * @return          True on success, false on error.
     */
    public native bool Log(const char[] logTime, const SourceLoc loc,
                           const char[] name, LogLevel lvl, const char[] msg,
                           char[] error="", int maxlen=0);

    /**
     * Manual flush sink contents.
     *
     * @param error     Buffer to store error description.
     * @param maxlen    Maximum length of the error.
     * @return          True on success, false on error.
     */
    public native bool Flush(char[] error="", int maxlen=0);

    /**
     * Gets the sink log level.
     *
     * @return          The sink log level.
     */
    public native LogLevel GetLevel();

    /**
     * Sets the sink log level.
     *
     * @param lvl       Log level enum.
     */
    public native void SetLevel(LogLevel lvl);

    /**
     * Gets whether logging is enabled for the given log level.
     *
     * @param lvl       Log level enum.
     * @return          True if logging is enabled for the given log level.
     */
    public native bool ShouldLog(LogLevel lvl);

    /**
     * Change log pattern.
     *
     * @note pattern flags: https://github.com/gabime/spdlog/wiki/Custom-formatting#pattern-flags
     *
     * @param pattern   Logger sinks pattern.
     * @param type      Pattern time.
     */
    public native void SetPattern(const char[] pattern, PatternTimeType type=PatternTimeType_Local);

    /**
     * Compares this Sink with the specified sink for equality.
     *
     * @param sink      Sink to which this sink is to be compared.
     * @return          True if the sinks are equal.
     */
    public native bool Equals(Sink sink);

    /**
     * Returns whether or not a sink handle is valid.
     *
     * @return          True if valid, false otherwise.
     */
    public native bool IsValid();

#else

    public bool Log(const char[] logTime, LOG4SP_ES_CONST SourceLoc loc,
                    const char[] name, LogLevel lvl, const char[] msg,
                    char[] error="", int maxlen=0) {
        if (!this.IsValid())
            ThrowError("Invalid Sink Handle %x.", this);

        PrivateForward logFwd = this.__GetLogFwd();
        if (logFwd) {
            Call_StartForward(logFwd);
            Call_PushCell(this);                // sink
            Call_PushStringEx(error, maxlen, SM_PARAM_STRING_COPY|SM_PARAM_STRING_UTF8, SM_PARAM_COPYBACK); // error
            Call_PushCell(maxlen);              // maxlen

            if (!logTime[0] || IsNullString(logTime)) {
                /**
                 * HACK: SourceMod can only retrieve timestamps in seconds.
                 *
                 * It appends 9 decimal places from the Engine Time timetamp to simulate a nanosecond timestamp.
                 * This isn't very accurate, but I think an error within 1 second is acceptable.
                 */
                int time64[2];
                GetTime(time64);

                // Time head
                char now[21];
                Int64ToString(time64, now, sizeof(now));

                int timeTail = RoundToNearest(FloatFraction(GetEngineTime()) * 1000000000);
                Format(now, sizeof(now), "%s%09d", now, timeTail);

                Call_PushString(now);           // logTime (now)
            } else {
                Call_PushString(logTime);       // logTime (User specified)
            }

            Call_PushArray(loc, sizeof(loc));   // loc
            Call_PushString(name);              // name
            Call_PushCell(lvl);                 // lvl
            Call_PushString(msg);               // msg

            bool result;
            Call_Finish(result);
            return result;
        }
        return true;
    }

    public bool Flush(char[] error="", int maxlen=0) {
        if (!this.IsValid())
            ThrowError("Invalid Sink Handle %x.", this);

        PrivateForward flushFwd = this.__GetFlushFwd();
        if (flushFwd) {
            Call_StartForward(flushFwd);
            Call_PushCell(this);                // sink
            Call_PushStringEx(error, maxlen, SM_PARAM_STRING_COPY|SM_PARAM_STRING_UTF8, SM_PARAM_COPYBACK); // error
            Call_PushCell(maxlen);              // maxlen

            bool result;
            Call_Finish(result);
            return result;
        }
        return true;
    }

    public LogLevel GetLevel() {
        if (!this.IsValid())
            ThrowError("Invalid Sink Handle %x.", this);
        return this.__GetLevel();
    }

    public void SetLevel(LogLevel lvl) {
        if (!this.IsValid())
            ThrowError("Invalid Sink Handle %x.", this);
        this.__SetLevel(lvl);
    }

    public bool ShouldLog(LogLevel lvl) {
        if (!this.IsValid())
            ThrowError("Invalid Sink Handle %x.", this);
        return lvl >= this.__GetLevel();
    }

    /**
     * @note Not implemented, do not use.
     */
    public void SetPattern(const char[] pattern, PatternTimeType type=PatternTimeType_Local) {
        #pragma unused pattern
        #pragma unused type
        // Using Forward to mimic spdlog::details::flag_formatter might achieve this.
        // However, it is difficult and complex to implement, and the efficiency will be much lower.
    }

    public Sink Clone() {
        if (!this.IsValid())
            ThrowError("Invalid Sink Handle %x.", this);

        int refCnt = this.__GetRefCnt() + 1;
        this.__SetRefCnt(refCnt);

        /**
         * After Plugin A calls "new *Sink()",
         * ownership of the nested handles in sink belongs to Plugin A.
         * For example: logFwd (PrivateForward), flushFwd (PrivateForward) ...
         *
         * "sink.Clone()" only increments the reference count,
         * it does not clone nested handles.
         *
         * Using "sink.Clone()" in Plugin B resolves the "Invalid Sink Handle"
         * that occurs after Plugin A calls "sink.Close()",
         * but it does not resolve the "Invalid Sink Handle"
         * that occurs after Plugin A is unloaded.
         */
        Sink cloned = view_as<Sink>(CloneHandle(this));
        return cloned;
    }

    public void Close() {
        if (!this.IsValid())
            ThrowError("Invalid Sink Handle %x.", this);

        int refCnt = this.__GetRefCnt() - 1;
        this.__SetRefCnt(refCnt);

        // Cleanup if no references
        if (refCnt <= 0) {
            // Call the Close forward of the derived methodmap
            // to cleanup their nested Handles.
            PrivateForward closeFwd = this.__GetCloseFwd();
            if (closeFwd) {
                Call_StartForward(closeFwd);
                Call_PushCell(this);            // sink
                Call_Finish();
            }

            // Cleanup all nested handles that the same ownership and lifecycle as this
            char owner[PLATFORM_MAX_PATH];
            this.__GetOwner(owner, sizeof(owner));
            Handle plugin = FindPluginByFile(owner);
            Function func = GetFunctionByName(plugin, "__Sink_OnClose");

            // void (Sink sink);
            Call_StartFunction(plugin, func);
            Call_PushCell(this);
            Call_Finish();

            // It will be destroyed soon, so ignore resetting it to null.
            // sink.__SetLogFwd(null);
            // sink.__SetFlushFwd(null);
            // sink.__SetCloseFwd(null);
        }
        delete view_as<StringMap>(this);
    }

    public bool Equals(Sink sink) {
        if (!this.IsValid())
            ThrowError("Invalid Sink Handle %x.", this);
        if (!sink.IsValid())
            ThrowError("Invalid Sink Handle %x.", sink);
        if (this == sink)
            return true;

        /**
         * Compare whether each property is the same.
         * The handle with the same ownership lifecycle as this
         * should be different for each Sink.
         */
        if (this.__GetRefCnt() != sink.__GetRefCnt()
         || this.__GetLevel()  != sink.__GetLevel()
         || this.__GetLogFwd() != sink.__GetLogFwd()
         || this.__GetFlushFwd() != sink.__GetFlushFwd()
         || this.__GetCloseFwd() != sink.__GetCloseFwd())
            return false;

        char buffer[2][PLATFORM_MAX_PATH];
        int len1 = this.__GetOwner(buffer[0], sizeof(buffer[]));
        int len2 = sink.__GetOwner(buffer[1], sizeof(buffer[]));
        if (len1 != len2 || !StrEqual(buffer[0], buffer[1]))
            return false;

        len1 = this.__GetClassname(buffer[0], sizeof(buffer[]));
        len2 = sink.__GetClassname(buffer[1], sizeof(buffer[]));
        if (len1 != len2 || !StrEqual(buffer[0], buffer[1]))
            return false;
        return true;
    }

    public bool IsValid() {
#if !defined LOG4SP_HEADER_ONLY_NO_VALID_HANDLE
        return !!this
         && this.__Contains("m_cRef")        && this.__Contains("m_szOwner")
         && this.__Contains("m_szClassname") && this.__Contains("m_eLvl")
         && this.__Contains("m_hLogFwd")     && this.__Contains("m_hFlushFwd")
         && this.__Contains("m_hCloseFwd");
#else
        return !!this;
#endif      // LOG4SP_HEADER_ONLY_NO_VALID_HANDLE
    }


    // *** Do not use the function below this line! *** //
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

    public any __GetRefCnt()                        { return this.__GetValue("m_cRef"); }
    public void __SetRefCnt(int refCnt)             { this.__SetValue("m_cRef", refCnt); }

    public int __GetOwner(char[] owner, int maxlen) { return this.__GetString("m_szOwner", owner, maxlen); }
    public void __SetOwner(const char[] owner)      { this.__SetString("m_szOwner", owner); }

    public int __GetClassname(char[] classname, int maxlen) { return this.__GetString("m_szClassname", classname, maxlen); }
    public void __SetClassname(const char[] classname)      { this.__SetString("m_szClassname", classname); }

    public LogLevel __GetLevel()                    { return this.__GetValue("m_eLvl"); }
    public void __SetLevel(LogLevel lvl)            { this.__SetValue("m_eLvl", lvl); }

    // Ownership are the same as this, closed in __Sink_OnClose().
    public PrivateForward __GetLogFwd()             { return this.__GetValue("m_hLogFwd"); }
    public void __SetLogFwd(PrivateForward fwd)     { this.__SetValue("m_hLogFwd", fwd); }

    // Ownership are the same as this, closed in __Sink_OnClose().
    public PrivateForward __GetFlushFwd()           { return this.__GetValue("m_hFlushFwd"); }
    public void __SetFlushFwd(PrivateForward fwd)   { this.__SetValue("m_hFlushFwd", fwd); }

    // Ownership are the same as this, closed in __Sink_OnClose().
    public PrivateForward __GetCloseFwd()           { return this.__GetValue("m_hCloseFwd"); }
    public void __SetCloseFwd(PrivateForward fwd)   { this.__SetValue("m_hCloseFwd", fwd); }

    // Use a derived methodmap instead of creating the Sink directly.
    public static Sink __Sink(const char[] classname,
                              Handle logPlugin,   Function logFunc,
                              Handle flushPlugin, Function flushFunc,
                              Handle closePlugin, Function closeFunc) {
        char owner[PLATFORM_MAX_PATH];
        GetPluginFilename(null, owner, sizeof(owner));

        PrivateForward logFwd;
        if (logFunc && logFunc != INVALID_FUNCTION) {
            // bool (Sink sink, char[] error, int maxlen, const char[] logTime,
            //       LOG4SP_ES_CONST SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg);
            logFwd = new PrivateForward(ET_Single,   Param_Cell,   Param_String, Param_Cell, Param_String,
                                        Param_Array, Param_String, Param_Cell,   Param_String);
            logFwd.AddFunction(logPlugin, logFunc);
        }

        PrivateForward flushFwd;
        if (flushFunc && flushFunc != INVALID_FUNCTION) {
            // bool (Sink sink, char[] error, int maxlen);
            flushFwd = new PrivateForward(ET_Single, Param_Cell, Param_String, Param_Cell);
            flushFwd.AddFunction(flushPlugin, flushFunc);
        }

        PrivateForward closeFwd;
        if (closeFunc && closeFunc != INVALID_FUNCTION) {
            // void (Sink sink);
            closeFwd = new PrivateForward(ET_Ignore, Param_Cell);
            closeFwd.AddFunction(closePlugin, closeFunc);
        }

        if (!logFwd && !flushFwd && !closeFwd)
            ThrowError("Invalid derived Sink methodmap.");

        Sink sink = view_as<Sink>(new StringMap());
        sink.__SetRefCnt(1);
        sink.__SetOwner(owner);
        sink.__SetClassname(classname);
        sink.__SetLevel(LogLevel_Trace);
        sink.__SetLogFwd(logFwd);
        sink.__SetFlushFwd(flushFwd);
        sink.__SetCloseFwd(closeFwd);
        return sink;
    }
#endif      // LOG4SP_HEADER_ONLY
}


#if defined LOG4SP_HEADER_ONLY
public void __Sink_OnClose(Sink sink)
{
    delete sink.__GetLogFwd();
    delete sink.__GetFlushFwd();
    delete sink.__GetCloseFwd();
}
#endif      // LOG4SP_HEADER_ONLY

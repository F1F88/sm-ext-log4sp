#if defined _log4sp_logger_included
    #endinput
#endif
#define _log4sp_logger_included

#include <log4sp/common>
#include <log4sp/sinks/sink>


/**
 * Cleans up a logger handle and sets the passed variable to null.
 *
 * @param logger    Logger handle to clean up.
 */
stock void LoggerCleanupAndDelete(Logger &logger)
{
    logger.Close();
    logger = null;
}

/**
 * Cleans up a logger handle and sets the passed variable to null.
 *
 * @param loggers   Logger handles to clean up.
 * @param maxlen    Maximum length of the loggers.
 */
stock void LoggersCleanupAndDelete(Logger[] loggers, int maxlen)
{
    for (int i = 0; i < maxlen; ++i)
    {
        loggers[i].Close();
        loggers[i] = null;
    }
}


/**
 * The Logger is the core component of the logging library. It receives log
 * messages, processes them, and dispatches them to one or more sinks.
 */
methodmap Logger < Handle
{
#if !defined LOG4SP_HEADER_ONLY
    /**
     * Creating a logger handle.
     *
     * @note Logger handles must be freed via delete or CloseHandle().
     *
     * @param name      The name of the new logger.
     *                  If NULL_STRING, defaults to the plugin filename.
     * @return          A new logger handle.
     */
    public native Logger(const char[] name=NULL_STRING);

    /**
     * Logs a message.
     *
     * @param lvl       Log level enum.
     * @param msg       Log message.
     */
    public native void Log(LogLevel lvl, const char[] msg);
    public native void LogF(LogLevel lvl, const char[] fmt, any ...);

    /**
     * Logs a message with the scripting source location.
     *
     * @param lvl       Log level enum.
     * @param msg       Log message.
     */
    public native void LogSrc(LogLevel lvl, const char[] msg);
    public native void LogSrcF(LogLevel lvl, const char[] fmt, any ...);

    /**
     * Logs a message with the given source location.
     *
     * @param loc       Source location.
     * @param lvl       Log level enum.
     * @param msg       Log message.
     */
    public native void LogLoc(const SourceLoc loc, LogLevel lvl, const char[] msg);
    public native void LogLocF(const SourceLoc loc, LogLevel lvl, const char[] fmt, any ...);

    /**
     * Logs a stack trace from the current function call.
     * Code execution continues after the call.
     *
     * @param lvl       Log level enum.
     * @param msg       Log message.
     */
    public native void LogStackTrace(LogLevel lvl, const char[] msg);
    public native void LogStackTraceF(LogLevel lvl, const char[] fmt, any ...);

    public native void Trace(const char[] msg);
    public native void TraceF(const char[] fmt, any ...);

    public native void Debug(const char[] msg);
    public native void DebugF(const char[] fmt, any ...);

    public native void Info(const char[] msg);
    public native void InfoF(const char[] fmt, any ...);

    public native void Warn(const char[] msg);
    public native void WarnF(const char[] fmt, any ...);

    public native void Error(const char[] msg);
    public native void ErrorF(const char[] fmt, any ...);

    public native void Fatal(const char[] msg);
    public native void FatalF(const char[] fmt, any ...);

    /**
     * Gets the logger name.
     *
     * @param buffer    Buffer to store logger name.
     * @param maxlen    Maximum length of the buffer.
     * @return          Number of bytes written.
     */
    public native int GetName(char[] buffer, int maxlen);

    /**
     * Gets the length of the logger name.
     *
     * @return          Length of the logger name string.
     */
    public native int GetNameLength();

    /**
     * Gets the logger log level.
     *
     * @return          The logger log level.
     */
    public native LogLevel GetLevel();

    /**
     * Sets the logger log level.
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
     * Sets formatting pattern for the sinks in the logger.
     *
     * @note each sink will get a new instance of a formatter object, replacing the old one.
     * @note pattern flags: https://github.com/gabime/spdlog/wiki/Custom-formatting#pattern-flags
     *
     * @param pattern   Formatting pattern.
     * @param type      Pattern time type.
     */
    public native void SetPattern(const char[] pattern, PatternTimeType type=PatternTimeType_Local);

    /**
     * Manual flush the contents for the sinks in the logger.
     */
    public native void Flush();

    /**
     * Gets the minimum log level that will trigger automatic flush.
     *
     * @return          Minimum log level that will trigger automatic flush.
     */
    public native LogLevel GetFlushLevel();

    /**
     * Sets the minimum log level that will trigger automatic flush.
     *
     * @param lvl       Log level enum.
     */
    public native void SetFlushLevel(LogLevel lvl);

    /**
     * Gets whether trigger automatic flush for the given log level.
     *
     * @param flushLvl  Log level enum.
     * @return          True if trigger automatic flush for the given log level.
     */
    public native bool ShouldFlush(LogLevel flushLvl);

    /**
     * Gets the sinks clone of logger.
     *
     * @note Sinks clone handles must be freed via delete or CloseHandle() or Sink.Close().
     *
     * @param sinks     Buffer to store sinks clone.
     * @param maxlen    Maximum length of the buffer.
     * @return          Number of sinks clone written.
     */
    public native int GetSinks(Sink[] sinks, int maxlen);

    /**
     * Gets the length of the logger sinks.
     *
     * @return          Length of the logger sinks array.
     */
    public native int GetSinksLength();

    /**
     * Clone the sink handle and add it to the logger.
     *
     * @param sink      Sink handle.
     * @error           Invalid Sink handle.
     */
    public native void AddSink(Sink sink);

    /**
     * Close the cloned sink handle and remove from logger.
     *
     * @param sink      Sink handle.
     * @error           Invalid Sink handle.
     */
    public native void DropSink(Sink sink);

    /**
     * Sets the handler function for logging errors.
     *
     * @param plugin    Handle of the plugin that contains the function.
     *                  If null, defaults to the calling plugin.
     * @param func      Callback function to call when an error occurs.
     * @error           Invalid or corrupt plugin handle, or invalid function.
     */
    public native void SetErrorHandler(Handle plugin, ErrorHandler func);

    /**
     * Compares this Logger with the specified logger for equality.
     *
     * @param logger    Logger to which this logger is to be compared.
     * @return          True if the loggers are equal.
     */
    public native bool Equals(Logger logger);

    /**
     * Returns whether or not a logger handle is valid.
     *
     * @return          True if valid, false otherwise.
     */
    public native bool IsValid();

#else

    /**
     * Creating a logger handle.
     *
     * @note Logger handles must be freed via logger.Close().
     * @note Multi-plugin shared handles must be freed before unload the plugin.
     *
     * @param name      The name of the new logger.
     *                  If NULL_STRING, defaults to the plugin filename.
     * @return          A new logger handle.
     */
    public Logger(const char[] name=NULL_STRING) {
        char owner[PLATFORM_MAX_PATH];
        GetPluginFilename(null, owner, sizeof(owner));

        Logger logger = view_as<Logger>(new StringMap());
        logger.__SetRefCnt(1);
        logger.__SetOwner(owner);
        logger.__SetName(IsNullString(name) ? owner : name);
        logger.__SetNameLength(IsNullString(name) ? strlen(owner) : strlen(name));
        logger.__SetLevel(LogLevel_Info);
        logger.__SetSinks(new ArrayList());
        logger.__SetSinkOwners(new ArrayList(.blocksize=PLATFORM_MAX_PATH));
        logger.__SetFlushLevel(LogLevel_Off);
        // function void (const char[] origin, LOG4SP_ES_CONST SourceLoc loc, const char[] msg);
        logger.__SetErrFwd(new PrivateForward(ET_Ignore, Param_String, Param_Array, Param_String));
        logger.__SetErrPlugin("");
        return logger;
    }

    public void Log(LogLevel lvl, const char[] msg) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        if (this.ShouldLog(lvl)) {
            SourceLoc loc;
            this.__LogIt(loc, lvl, msg);
        }
    }
    public void LogF(LogLevel lvl, const char[] fmt, any ...) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        if (this.ShouldLog(lvl)) {
            SourceLoc loc;
            char msg[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];
            VFormat(msg, sizeof(msg), fmt, 4);
            this.__LogIt(loc, lvl, msg);
        }
    }

    public void LogSrc(LogLevel lvl, const char[] msg) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        if (this.ShouldLog(lvl))
            this.__LogIt(this.__GetSourceLoc(1), lvl, msg);
    }
    public void LogSrcF(LogLevel lvl, const char[] fmt, any ...) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        if (this.ShouldLog(lvl)) {
            char msg[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];
            VFormat(msg, sizeof(msg), fmt, 4);
            this.__LogIt(this.__GetSourceLoc(1), lvl, msg);
        }
    }

    public void LogLoc(LOG4SP_ES_CONST SourceLoc loc, LogLevel lvl, const char[] msg) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        if (this.ShouldLog(lvl))
            this.__LogIt(loc, lvl, msg);
    }

    public void LogLocF(LOG4SP_ES_CONST SourceLoc loc, LogLevel lvl, const char[] fmt, any ...) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        if (this.ShouldLog(lvl)) {
            char msg[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];
            VFormat(msg, sizeof(msg), fmt, 5);
            this.__LogIt(loc, lvl, msg);
        }
    }

    public void LogStackTrace(LogLevel lvl, const char[] msg) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        if (this.ShouldLog(lvl)) {
            SourceLoc loc;
            loc = this.__GetSourceLoc(1);

            char buffer[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];
            FormatEx(buffer, sizeof(buffer), "Stack trace requested: %s", msg);
            this.__LogIt(loc, lvl, buffer);

            char filename[PLATFORM_MAX_PATH];
            GetPluginFilename(null, filename, sizeof(filename));
            FormatEx(buffer, sizeof(buffer), "Called from: %s", filename);
            this.__LogIt(loc, lvl, buffer);

            FrameIterator frame = new FrameIterator();
            ArrayList arr = this.__GetStackTrace(frame);
            delete frame;

            int length = arr.Length;
            for (int i = 0; i < length; ++i) {
                arr.GetString(i, buffer, sizeof(buffer));
                this.__LogIt(loc, lvl, buffer);
            }
            delete arr;
        }
    }
    public void LogStackTraceF(LogLevel lvl, const char[] fmt, any ...) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        if (this.ShouldLog(lvl)) {
            SourceLoc loc;
            loc = this.__GetSourceLoc(1);

            char msg[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];
            VFormat(msg, sizeof(msg), fmt, 4);

            char buffer[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];
            FormatEx(buffer, sizeof(buffer), "Stack trace requested: %s", msg);
            this.__LogIt(loc, lvl, buffer);

            char filename[PLATFORM_MAX_PATH];
            GetPluginFilename(null, filename, sizeof(filename));
            FormatEx(buffer, sizeof(buffer), "Called from: %s", filename);
            this.__LogIt(loc, lvl, buffer);

            FrameIterator frame = new FrameIterator();
            ArrayList arr = this.__GetStackTrace(frame);
            delete frame;

            int length = arr.Length;
            for (int i = 0; i < length; ++i) {
                arr.GetString(i, buffer, sizeof(buffer));
                this.__LogIt(loc, lvl, buffer);
            }
            delete arr;
        }
    }

    public void Trace(const char[] msg) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        if (this.ShouldLog(LogLevel_Trace)) {
            SourceLoc loc;
            this.__LogIt(loc, LogLevel_Trace, msg);
        }
    }
    public void TraceF(const char[] fmt, any ...) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        if (this.ShouldLog(LogLevel_Trace)) {
            SourceLoc loc;
            char msg[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];
            VFormat(msg, sizeof(msg), fmt, 3);
            this.__LogIt(loc, LogLevel_Trace, msg);
        }
    }

    public void Debug(const char[] msg) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        if (this.ShouldLog(LogLevel_Debug)) {
            SourceLoc loc;
            this.__LogIt(loc, LogLevel_Debug, msg);
        }
    }
    public void DebugF(const char[] fmt, any ...) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        if (this.ShouldLog(LogLevel_Debug)) {
            SourceLoc loc;
            char msg[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];
            VFormat(msg, sizeof(msg), fmt, 3);
            this.__LogIt(loc, LogLevel_Debug, msg);
        }
    }

    public void Info(const char[] msg) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        if (this.ShouldLog(LogLevel_Info)) {
            SourceLoc loc;
            this.__LogIt(loc, LogLevel_Info, msg);
        }
    }
    public void InfoF(const char[] fmt, any ...) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        if (this.ShouldLog(LogLevel_Info)) {
            SourceLoc loc;
            char msg[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];
            VFormat(msg, sizeof(msg), fmt, 3);
            this.__LogIt(loc, LogLevel_Info, msg);
        }
    }

    public void Warn(const char[] msg) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        if (this.ShouldLog(LogLevel_Warn)) {
            SourceLoc loc;
            this.__LogIt(loc, LogLevel_Warn, msg);
        }
    }
    public void WarnF(const char[] fmt, any ...) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        if (this.ShouldLog(LogLevel_Warn)) {
            SourceLoc loc;
            char msg[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];
            VFormat(msg, sizeof(msg), fmt, 3);
            this.__LogIt(loc, LogLevel_Warn, msg);
        }
    }

    public void Error(const char[] msg) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        if (this.ShouldLog(LogLevel_Error)) {
            SourceLoc loc;
            this.__LogIt(loc, LogLevel_Error, msg);
        }
    }
    public void ErrorF(const char[] fmt, any ...) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        if (this.ShouldLog(LogLevel_Error)) {
            SourceLoc loc;
            char msg[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];
            VFormat(msg, sizeof(msg), fmt, 3);
            this.__LogIt(loc, LogLevel_Error, msg);
        }
    }

    public void Fatal(const char[] msg) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        if (this.ShouldLog(LogLevel_Fatal)) {
            SourceLoc loc;
            this.__LogIt(loc, LogLevel_Fatal, msg);
        }
    }
    public void FatalF(const char[] fmt, any ...) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        if (this.ShouldLog(LogLevel_Fatal)) {
            SourceLoc loc;
            char msg[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];
            VFormat(msg, sizeof(msg), fmt, 3);
            this.__LogIt(loc, LogLevel_Fatal, msg);
        }
    }

    public int GetName(char[] buffer, int maxlen) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);
        return this.__GetName(buffer, maxlen);
    }

    public int GetNameLength() {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);
        return this.__GetNameLength();
    }

    public LogLevel GetLevel() {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);
        return this.__GetLevel();
    }

    public void SetLevel(LogLevel lvl) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);
        this.__SetLevel(lvl);
    }

    public bool ShouldLog(LogLevel lvl) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);
        return lvl >= this.__GetLevel();
    }

    /**
     * @note sink.SetPattern() does not implemented, do not use.
     */
    public void SetPattern(const char[] pattern, PatternTimeType type=PatternTimeType_Local) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);
        for (int i = 0; i < this.__GetSinks().Length; ++i)
            view_as<Sink>(this.__GetSinks().Get(i)).SetPattern(pattern, type);
    }

    public void Flush() {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);
        this.__FlushIt(this.__GetSourceLoc(1));
    }

    public LogLevel GetFlushLevel() {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);
        return this.__GetFlushLevel();
    }

    public void SetFlushLevel(LogLevel flushLvl) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);
        this.__SetFlushLevel(flushLvl);
    }

    public bool ShouldFlush(LogLevel flushLvl) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);
        return (flushLvl >= this.__GetFlushLevel()) && flushLvl != LogLevel_Off;
    }

    /**
     * Gets the sinks clone of logger.
     *
     * @note Sinks clone handles must be freed via Sink.Close().
     *
     * @param sinks     Buffer to store sinks clone.
     * @param maxlen    Maximum length of the buffer.
     * @return          Number of sinks clone written.
     */
    public int GetSinks(Sink[] sinks, int maxlen) {
        int counter = 0;
        while (counter < maxlen) {
            // To prevent the internal clone from being destroyed by the outside,
            // we return the clone of the internal clone.
            // Ownership belongs to an external party,
            // and the external party must manually shut it down.
            Sink sink = view_as<Sink>(this.__GetSinks().Get(counter));
            sinks[counter] = sink.Clone();
            ++counter;
        }
        return counter;
    }

    public int GetSinksLength() {
        return this.__GetSinks().Length;
    }

    public void AddSink(Sink sink) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);
        if (!sink.IsValid())
            ThrowError("Invalid Sink Handle %x.", sink);

        // Save the identity of the adder
        char owner[PLATFORM_MAX_PATH];
        GetPluginFilename(null, owner, sizeof(owner));

        // Cloning and then adding ensures that closing the original sink does
        // not affect the sink held by the logger.
        // Clones cannot be obtained externally;
        // they can only be closed using "logger.Drop()".
        this.__GetSinks().Push(sink.Clone());
        this.__GetSinkOwners().PushString(owner);
    }

    public void DropSink(Sink sink) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);
        if (!sink.IsValid())
            ThrowError("Invalid Sink Handle %x.", sink);

        // Remove and close
        for (int i = 0; i < this.__GetSinks().Length; ++i) {
            Sink cloned = view_as<Sink>(this.__GetSinks().Get(i));
            if (!cloned.Equals(sink))
                continue;

            // Close as the adder
            char owner[PLATFORM_MAX_PATH];
            this.__GetSinkOwners().GetString(i, owner, sizeof(owner));
            Handle plugin = FindPluginByFile(owner);
            Function func = GetFunctionByName(plugin, "__Logger_OnDropSink");

            // void (Sink sink);
            Call_StartFunction(plugin, func);
            Call_PushCell(cloned);
            Call_Finish();

            // Remove
            this.__GetSinks().Erase(i);
            this.__GetSinkOwners().Erase(i);
            --i;
        }
    }

    public void SetErrorHandler(Handle plugin, ErrorHandler func) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        // Cleanup old function
        char filename[PLATFORM_MAX_PATH];
        this.__GetErrPlugin(filename, sizeof(filename));
        if (filename[0]) {
            Handle errPlugin = FindPluginByFile(filename);
            if (errPlugin)
                this.__GetErrFwd().RemoveAllFunctions(errPlugin);
        }

        // Set a new function that will throw an error if it fails.
        GetPluginFilename(plugin, filename, sizeof(filename));
        this.__GetErrFwd().AddFunction(plugin, func);
        this.__SetErrPlugin(filename);
    }

    public Logger Clone() {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        int refCnt = this.__GetRefCnt() + 1;
        this.__SetRefCnt(refCnt);

        /**
         * After Plugin A calls "new Logger()",
         * ownership of the nested handles in logger belongs to Plugin A.
         * For example: sinks (ArrayList), ErrorHandler (Forward)
         *
         * "logger.Clone()" only increments the reference count,
         * it does not clone nested handles.
         *
         * Using "logger.Clone()" in Plugin B resolves the "Invalid Logger Handle"
         * that occurs after Plugin A calls "logger.Close()",
         * but it does not resolve the "Invalid Logger Handle"
         * that occurs after Plugin A is unloaded.
         */
        Logger cloned = view_as<Logger>(CloneHandle(this));
        return cloned;
    }

    public void Close() {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);

        int refCnt = this.__GetRefCnt() - 1;
        this.__SetRefCnt(refCnt);

        // Cleanup if no references
        if (refCnt <= 0) {
            // Cleanup all sinks
            for (int i = this.__GetSinks().Length - 1; i >= 0 ; --i) {
                this.DropSink(view_as<Sink>(this.__GetSinks().Get(i)));
            }

            // Cleanup all nested handles that the same ownership and lifecycle as this
            char owner[PLATFORM_MAX_PATH];
            this.__GetOwner(owner, sizeof(owner));
            Handle plugin = FindPluginByFile(owner);
            Function func = GetFunctionByName(plugin, "__Logger_OnClose");

            // void (Logger logger);
            Call_StartFunction(plugin, func);
            Call_PushCell(this);
            Call_Finish();

            // It will be destroyed soon, so ignore resetting it to null.
            // logger.__SetSinks(null);
            // logger.__SetSinkOwners(null);
            // logger.__SetErrorFwd(null);
        }
        delete view_as<StringMap>(this);
    }

    public bool Equals(Logger logger) {
        if (!this.IsValid())
            ThrowError("Invalid Logger Handle %x.", this);
        if (!logger.IsValid())
            ThrowError("Invalid Logger Handle %x.", logger);
        if (this == logger)
            return true;

        /**
         * Compare whether each property is the same.
         * The handle with the same ownership lifecycle as this
         * should be different for each Logger.
         */
        if (this.__GetRefCnt() != logger.__GetRefCnt()
         || this.__GetLevel() != logger.__GetLevel()
         || this.__GetFlushLevel() != logger.__GetFlushLevel()
         || this.__GetErrFwd() != logger.__GetErrFwd())
            return false;

        char buffer[2][PLATFORM_MAX_PATH];
        int len1 = this.__GetOwner(buffer[0], sizeof(buffer[]));
        int len2 = logger.__GetOwner(buffer[1], sizeof(buffer[]));
        if (len1 != len2 || !StrEqual(buffer[0], buffer[1]))
            return false;

        int nameLen1 = this.__GetNameLength() + 1;
        int nameLen2 = logger.__GetNameLength() + 1;
        if (nameLen1 != nameLen2)
            return false;

        char[] name1 = new char[nameLen1];
        char[] name2 = new char[nameLen2];

        len1 = this.__GetName(name1, nameLen1);
        len2 = logger.__GetName(name2, nameLen2);
        if (len1 != len2 || !StrEqual(name1, name2))
            return false;

        ArrayList sinks1 = this.__GetSinks();
        ArrayList sinks2 = logger.__GetSinks();
        if (sinks1 != sinks2 || sinks1.Length != sinks2.Length)
            return false;

        ArrayList owners1 = this.__GetSinkOwners();
        ArrayList owners2 = logger.__GetSinkOwners();
        if (owners1 != owners2 || owners1.Length != owners2.Length)
            return false;

        for (int i = 0; i < sinks1.Length; ++i) {
            Sink sink1 = view_as<Sink>(sinks1.Get(i));
            Sink sink2 = view_as<Sink>(sinks2.Get(i));
            if (!sink1.Equals(sink2))
                return false;

            len1 = owners1.GetString(i, buffer[0], sizeof(buffer[]));
            len2 = owners2.GetString(i, buffer[1], sizeof(buffer[]));
            if (len1 != len2 || !StrEqual(buffer[0], buffer[1]))
                return false;
        }

        len1 = this.__GetErrPlugin(buffer[0], sizeof(buffer[]));
        len2 = logger.__GetErrPlugin(buffer[1], sizeof(buffer[]));
        if (len1 != len2 || !StrEqual(buffer[0], buffer[1]))
            return false;
        return true;
    }

    public bool IsValid() {
#if !defined LOG4SP_HEADER_ONLY_NO_VALID_HANDLE
        return this
         && this.__Contains("m_cRef")
         && this.__Contains("m_szOwner")    && this.__Contains("m_szName")
         && this.__Contains("m_uNameLen")   && this.__Contains("m_eLvl")
         && this.__Contains("m_hSinks")     && this.__Contains("m_hSinkOwners")
         && this.__Contains("m_eFlushLvl")  && this.__Contains("m_hErrFwd")
         && this.__Contains("m_szErrPlugin");
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

    /**
     * Initial value = 1
     * After logger.Clone() + 1
     * After logger.Close() - 1
     * If (value <= 0) cleanup handle
     */
    public any __GetRefCnt()                        { return this.__GetValue("m_cRef"); }
    public void __SetRefCnt(int refCnt)             { this.__SetValue("m_cRef", refCnt); }

    /**
     * After Plugin A calls "new Logger()",
     * ownership of the nested handles in logger belongs to Plugin A.
     * For example: sinks (ArrayList), ErrorHandler (Forward)
     *
     * If Plugin B calls "logger.Close()" and refCnt <= 0,
     * the nested handles cannot be close using Plugin B identity.
     *
     * Nested handles need to be close as Plugin A.
     *
     * Owner is used to store and retrieve Plugin A.
     * Ultimately, calling Plugin A's __Logger_OnClose() cleanup the nested handle.
     */
    public int __GetOwner(char[] owner, int maxlen) { return this.__GetString("m_szOwner", owner, maxlen); }
    public void __SetOwner(const char[] owner)      { this.__SetString("m_szOwner", owner); }

    public int __GetName(char[] name, int maxlen)   { return this.__GetString("m_szName", name, maxlen); }
    public void __SetName(const char[] name)        { this.__SetString("m_szName", name); }

    public int __GetNameLength()                    { return this.__GetValue("m_uNameLen"); }
    public void __SetNameLength(int nameLength)     { this.__SetValue("m_uNameLen", nameLength); }

    public LogLevel __GetLevel()                    { return this.__GetValue("m_eLvl"); }
    public void __SetLevel(LogLevel lvl)            { this.__SetValue("m_eLvl", lvl); }

    /**
     * Ownership and lifecycle are the same as this, closed in __Logger_OnClose().
     * Ownership and lifecycle of elements are uncertain,
     * they are closed individually based on SinkOwners.
     */
    public ArrayList __GetSinks()                   { return this.__GetValue("m_hSinks"); }
    public void __SetSinks(ArrayList sinks)         { this.__SetValue("m_hSinks", sinks); }

    /**
     * After plugin A calls "new Sink() & logger.AddSink(sink)",
     * ownership of the sink handle cloned by logger belongs to Plugin A.
     *
     * If Plugin B calls "logger.Close()" and refCnt <= 0,
     * the sink handle cannot be close using plugin B identity.
     *
     * Sink handle need to be close as plugin A.
     *
     * SinkOwners is used to store and retrieve Plugin A.
     * Ultimately, calling Plugin A's __Logger_OnDropSink() cleanup the sink handle.
     *
     * Ownership and lifecycle are the same as this, closed in __Logger_OnClose().
     */
    public ArrayList __GetSinkOwners()              { return this.__GetValue("m_hSinkOwners"); }
    public void __SetSinkOwners(ArrayList owners)   { this.__SetValue("m_hSinkOwners", owners); }

    public LogLevel __GetFlushLevel()               { return this.__GetValue("m_eFlushLvl"); }
    public void __SetFlushLevel(LogLevel flushLvl)  { this.__SetValue("m_eFlushLvl", flushLvl); }

    // Ownership and lifecycle are the same as this, closed in __Logger_OnClose().
    public PrivateForward __GetErrFwd()             { return this.__GetValue("m_hErrFwd"); }
    public void __SetErrFwd(PrivateForward fwd)     { this.__SetValue("m_hErrFwd", fwd); }

    /**
     * Save the plugin associated with the current error handler function.
     * When setting a new error handler function, remove the old one.
     */
    public int __GetErrPlugin(char[] plugin, int maxlen){ return this.__GetString("m_szErrPlugin", plugin, maxlen); }
    public void __SetErrPlugin(const char[] plugin)     { this.__SetString("m_szErrPlugin", plugin); }

    public void __LogIt(LOG4SP_ES_CONST SourceLoc loc, LogLevel lvl, const char[] msg) {
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

        // Logger name
        int maxlen = this.__GetNameLength() + 1;
        char[] name = new char[maxlen];
        this.__GetName(name, maxlen);

        // Iterate through all sinks, and sink.Log() if allowed.
        ArrayList sinks = this.__GetSinks();
        int length = sinks.Length;
        char error[LOG4SP_HEADER_ONLY_MAX_ERR_LENGTH];
        for (int i = 0; i < length; ++i) {
            Sink sink = sinks.Get(i);
            if (!sink.ShouldLog(lvl))
                continue;

            // Handle the error to ErrorHandler and continue executing the next sink.Log()
            if (!sink.Log(now, loc, name, lvl, msg, error, sizeof(error)))
                this.__HandlerError(name, loc.IsEmpty() ? this.__GetSourceLoc(2) : loc, error);
        }

        // If allowed, execution Sink.Flush for traversing all sinks.
        if (this.ShouldFlush(lvl))
            this.__FlushIt(loc);
    }

    public void __FlushIt(LOG4SP_ES_CONST SourceLoc loc) {
        ArrayList sinks = this.__GetSinks();
        int length = sinks.Length;
        char error[LOG4SP_HEADER_ONLY_MAX_ERR_LENGTH];
        for (int i = 0; i < length; ++i) {
            Sink sink = sinks.Get(i);
            // Handle the error to ErrorHandler and continue executing the next sink.Flush()
            if (!sink.Flush(error, sizeof(error))) {
                int maxlen = this.__GetNameLength() + 1;
                char[] name = new char[maxlen];
                this.__GetName(name, maxlen);
                this.__HandlerError(name, loc.IsEmpty() ? this.__GetSourceLoc(3) : loc, error);
            }
        }
    }

    public void __HandlerError(const char[] origin, LOG4SP_ES_CONST SourceLoc loc, const char[] msg) {
        PrivateForward errFwd = this.__GetErrFwd();
        if (errFwd.FunctionCount) {
            Call_StartForward(errFwd);
            Call_PushString(origin);            // origin
            Call_PushArray(loc, sizeof(loc));   // loc
            Call_PushString(msg);               // msg
            Call_Finish();
        } else {
            // Default logging to SM's error log file
            char source[256];
            if (!loc.IsEmpty()) {
                int sepOffset = 0;
                for (int i = 0; i < sizeof(SourceLoc::filename); ++i) {
                    if (!loc.filename[i])
                        break;
                    if (loc.filename[i] == '\\' || loc.filename[i] == '/')
                        sepOffset = i + 1;
                }
                FormatEx(source, sizeof(source), "[%s::%u] ", loc.filename[sepOffset], loc.line);
            }
            LogError("%s[%s] %s", source, origin, msg);
        }
    }

    public SourceLoc __GetSourceLoc(int n=0) {
        /**
         *      FrameIterator.FrameIterator
         *      Logger.__GetSourceLoc   | ...\scripting\include\log4sp\logger.inc
         * 3 |  Logger.__FlushIt        | ...\scripting\include\log4sp\logger.inc
         * 2 |  Logger.__LogIt          | ...\scripting\include\log4sp\logger.inc
         * 1 |  Logger.Info             | ...\scripting\include\log4sp\logger.inc
         * 0 |  Command_Test            | ...\scripting\test-abcd.sp
         */
        FrameIterator frame = new FrameIterator();
        frame.Next(); frame.Next();
        for (int i = 0; i < n && frame.Next(); ++i) {}

        SourceLoc loc;
        loc.line = frame.LineNumber;
        frame.GetFilePath(loc.filename, sizeof(SourceLoc::filename));
        frame.GetFunctionName(loc.funcname, sizeof(SourceLoc::funcname));
        delete frame;
        return loc;
    }

    // https://github.com/alliedmodders/sourcemod/blob/fba9d713b70caf43bcf824dfcf1f7c2722a42a68/core/logic/DebugReporter.cpp#L165
    public ArrayList __GetStackTrace(FrameIterator frame) {
        char buffer[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];
        char funcname[sizeof(SourceLoc::funcname)];
        char filename[sizeof(SourceLoc::filename)];
        ArrayList trace = new ArrayList(ByteCountToCells(sizeof(buffer)));
        strcopy(buffer, sizeof(buffer), "Call stack trace:");
        trace.PushString(buffer);

        frame.Reset(); frame.Next();
        frame.GetFunctionName(funcname, sizeof(funcname));
        FormatEx(buffer, sizeof(buffer), "  [0] %s", funcname);
        trace.PushString(buffer);

        for (int index = 1; frame.Next(); ++index) {
            frame.GetFunctionName(funcname, sizeof(funcname));
            if (!funcname[0])
                continue;

            int line = frame.LineNumber;
            if (line == 0) {
                FormatEx(buffer, sizeof(buffer), "  [%d] %s", index, funcname);
                trace.PushString(buffer);
            } else {
                frame.GetFilePath(filename, sizeof(filename));
                FormatEx(buffer, sizeof(buffer), "  [%d] Line %d, %s::%s", index, line, filename, funcname);
                trace.PushString(buffer);
            }
        }
        return trace;
    }
#endif      // LOG4SP_HEADER_ONLY
}


#if defined LOG4SP_HEADER_ONLY
public void __Logger_OnDropSink(Sink sink)
{
    sink.Close();
}

// https://github.com/alliedmodders/sourcemod/issues/1041
// The fwd handle cannot be closed directly in the callback function
public void __Logger_OnClose(Logger logger)
{
    delete logger.__GetSinks();
    delete logger.__GetSinkOwners();
    delete logger.__GetErrFwd();
}
#endif      // LOG4SP_HEADER_ONLY

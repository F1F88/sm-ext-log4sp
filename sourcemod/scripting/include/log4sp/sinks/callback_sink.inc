#if defined _log4sp_sinks_callback_sink_included
    #endinput
#endif
#define _log4sp_sinks_callback_sink_included

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
    LOG4SP_ES_CONST SourceLoc loc,
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
#if !defined LOG4SP_HEADER_ONLY
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

#else

    /**
     * Trivial callback sink, gets a callback function and calls it on each log or flush.
     *
     * @note CallbackSink handles must be freed via Sink.Close().
     * @note Multi-plugin shared Handles must be freed before unload the plugin.
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
    public CallbackSink(Handle logPlugin, CallbackSinkLogFunction logFunc,
                        Handle flushPlugin, CallbackSinkFlushFunction flushFunc,
                        Handle closePlugin, CallbackSinkCloseFunction closeFunc,
                        any data=0) {
        PrivateForward closeFwd;
        if (closeFunc != INVALID_FUNCTION) {
            // void (any data);
            closeFwd = new PrivateForward(ET_Ignore, Param_Cell);
            closeFwd.AddFunction(closePlugin, closeFunc);
        }

        CallbackSink sink = view_as<CallbackSink>(Sink.__Sink("CallbackSink",
                                                              logPlugin, logFunc,
                                                              flushPlugin, flushFunc,
                                                              null, __CallbackSink_OnClose));
        sink.__SetData(data);
        sink.__SetCustomCloseFwd(closeFwd);
        return sink;
    }

    public any GetData() {
        if (!this.IsValid())
            ThrowError("Invalid CallbackSink Handle %x.", this);
        return this.__GetData();
    }

    public void SetData(any data) {
        if (!this.IsValid())
            ThrowError("Invalid CallbackSink Handle %x.", this);
        this.__SetData(data);
    }

    public bool IsValid() {
        if (!view_as<Sink>(this).IsValid())
            return false;

#if !defined LOG4SP_HEADER_ONLY_NO_VALID_HANDLE
        if (!this.__Contains("m_vData") || !this.__Contains("m_hCustomCloseFwd"))
            return false;
#endif      // LOG4SP_HEADER_ONLY_NO_VALID_HANDLE

        char classname[16];
        this.__GetClassname(classname, sizeof(classname));
        return StrEqual(classname, "CallbackSink");
    }


    // *** Do not use the function below this line! *** //
    public any __GetData()                              { return this.__GetValue("m_vData"); }
    public void __SetData(any data)                     { this.__SetValue("m_vData", data); }

    // Ownership and lifecycle are the same as this, closed in OnClose.
    public PrivateForward __GetCustomCloseFwd()         { return this.__GetValue("m_hCustomCloseFwd"); }
    public void __SetCustomCloseFwd(PrivateForward fwd) { this.__SetValue("m_hCustomCloseFwd", fwd); }
#endif      // LOG4SP_HEADER_ONLY
}


#if defined LOG4SP_HEADER_ONLY
public void __CallbackSink_OnClose(CallbackSink sink)
{
    PrivateForward customCloseFwd = sink.__GetCustomCloseFwd();
    if (customCloseFwd)
    {
        any data = sink.__GetData();
        Call_StartForward(customCloseFwd);
        Call_PushCell(data);
        Call_Finish();
        delete customCloseFwd;

        // It will be destroyed soon, so ignore resetting it to null.
        // sink.__SetCustomCloseFwd(null);
    }
}
#endif      // LOG4SP_HEADER_ONLY

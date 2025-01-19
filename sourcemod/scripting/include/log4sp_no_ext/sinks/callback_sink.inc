#if defined _log_for_sourcepawn_sinks_callback_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_callback_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp_no_ext/sinks/sink>


typeset CustomLogCallback
{
    function void(const char[] name, LogLevel lvl, const char[] msg);
    function void(const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func);
    function void(const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int seconds[2]);
    function void(const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int seconds[2], int nanoseconds[2]);
}

typedef CustomLogPostCallback = function void (const char[] msg);

typedef CustomFlushCallback = function void();


methodmap CallbackSink < Sink
{
    public CallbackSink(CustomLogCallback logCallback = INVALID_FUNCTION, CustomLogPostCallback logPostCallback = INVALID_FUNCTION, CustomFlushCallback flushCallback = INVALID_FUNCTION) {
        return view_as<CallbackSink>(INVALID_HANDLE);
    }

    public void SetLogCallback(CustomLogCallback logCallback) {}

    public void SetLogPostCallback(CustomLogPostCallback logPostCallback) {}

    public void SetFlushCallback(CustomFlushCallback flushCallback) {}

    public int ToPattern(char[] buffer, int maxlen, const char[] name, LogLevel lvl, const char[] msg, const char[] file = NULL_STRING, int line = 0, const char[] func = NULL_STRING, int seconds[2] = {0, 0}, int nanoseconds[2] = {0, 0}) {
        return 0;
    }

    public static Logger CreateLogger(const char[] name, CustomLogCallback logCallback = INVALID_FUNCTION, CustomLogPostCallback logPostCallback = INVALID_FUNCTION, CustomFlushCallback flushCallback = INVALID_FUNCTION) {
        return null;
    }
}

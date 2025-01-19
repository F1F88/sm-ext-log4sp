#if defined _log_for_sourcepawn_sinks_ringbuffer_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_ringbuffer_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>


typeset DrainCallback
{
    function void (const char[] name, LogLevel lvl, const char[] msg);
    function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func);
    function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int seconds[2]);
    function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int seconds[2], int nanoseconds[2]);
    function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int seconds[2], int nanoseconds[2], any data);
}

typeset DrainFormattedCallback
{
    function void (const char[] msg);
    function void (const char[] msg, any data);
}


methodmap RingBufferSink < Sink
{
    public RingBufferSink(int amount) {
        return view_as<RingBufferSink>(INVALID_HANDLE);
    }

    public void Drain(DrainCallback callback, any data = 0) {}

    public void DrainFormatted(DrainFormattedCallback callback, any data = 0) {}

    public int ToPattern(char[] buffer, int maxlen, const char[] name, LogLevel lvl, const char[] msg, const char[] file = NULL_STRING, int line = 0, const char[] func = NULL_STRING, int seconds[2] = {0, 0}, int nanoseconds[2] = {0, 0}) {
        return 0;
    }

    public static Logger CreateLogger(const char[] name, int amount) {
        return null;
    }
}

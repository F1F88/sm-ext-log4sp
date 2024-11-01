#if defined _log_for_sourcepawn_sinks_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp_empty/common>


methodmap Sink < Handle
{
    public LogLevel GetLevel() { return LogLevel_Off; }

    public void SetLevel(LogLevel lvl) {}

    public void SetPattern(const char[] pattern) {}

    public bool ShouldLog(LogLevel lvl) { return false; }

    public void Log(const char[] name, LogLevel lvl, const char[] msg) {}
    public void LogAmxTpl(const char[] name, LogLevel lvl, const char[] fmt, any ...) {}
    // public void LogFmtTpl(LogLevel lvl, const char[] fmt, any ...) {}

    public void LogSrc(const char[] name, LogLevel lvl, const char[] msg) {}
    public void LogSrcAmxTpl(const char[] name, LogLevel lvl, const char[] fmt, any ...) {}
    // public void LogSrcFmtTpl(LogLevel lvl, const char[] fmt, any ...) {}

    public void LogLoc(const char[] file, int line, const char[] func, const char[] name, LogLevel lvl, const char[] msg) {}
    public void LogLocAmxTpl(const char[] file, int line, const char[] func, const char[] name, LogLevel lvl, const char[] fmt, any ...) {}
    // public void LogLocFmtTpl(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...) {}

    public void Flush() {}
}

#if defined _log_for_sourcepawn_logger_included
 #endinput
#endif
#define _log_for_sourcepawn_logger_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp_empty/common>
#include <log4sp_empty/sinks/sink>


typeset Log4spErrorCallback
{
    function void (const char[] msg);
};

methodmap Logger < Handle
{
    public Logger(const char[] name, Sink[] sinks, int numSinks, bool async=false, AsyncOverflowPolicy policy=AsyncOverflowPolicy_Block) { return view_as<Logger>(INVALID_HANDLE); }

    public static Logger Get(const char[] name) { return view_as<Logger>(INVALID_HANDLE); }

    public static Logger CreateServerConsoleLogger(const char[] name, bool async=false, AsyncOverflowPolicy policy=AsyncOverflowPolicy_Block) { return view_as<Logger>(INVALID_HANDLE); }

    public static Logger CreateBaseFileLogger(const char[] name, const char[] file, bool truncate=false, bool async=false, AsyncOverflowPolicy policy=AsyncOverflowPolicy_Block) { return view_as<Logger>(INVALID_HANDLE); }

    public static Logger CreateRotatingFileLogger(const char[] name, const char[] file, int maxFileSize, int maxFiles, bool rotateOnOpen=false, bool async=false, AsyncOverflowPolicy policy=AsyncOverflowPolicy_Block) { return view_as<Logger>(INVALID_HANDLE); }

    public static Logger CreateDailyFileLogger(const char[] name, const char[] file, int hour=0, int minute=0, bool truncate=false, int maxFiles=0, bool async=false, AsyncOverflowPolicy policy=AsyncOverflowPolicy_Block) { return view_as<Logger>(INVALID_HANDLE); }

    public void GetName(char[] buffer, int maxlen) {}

    public LogLevel GetLevel() { return LogLevel_Off; }

    public void SetLevel(LogLevel lvl) {}

    public void SetPattern(const char[] pattern, PatternTimeType type=PatternTimeType_Local) {}

    public bool ShouldLog(LogLevel lvl) { return false; }

    public void Log(LogLevel lvl, const char[] msg) {}
    public void LogAmxTpl(LogLevel lvl, const char[] fmt, any ...) {}
    // public void LogFmtTpl(LogLevel lvl, const char[] fmt, any ...) {}

    public void LogSrc(LogLevel lvl, const char[] msg) {}
    public void LogSrcAmxTpl(LogLevel lvl, const char[] fmt, any ...) {}
    // public void LogSrcFmtTpl(LogLevel lvl, const char[] fmt, any ...) {}

    public void LogLoc(const char[] file, int line, const char[] func, LogLevel lvl, const char[] msg) {}
    public void LogLocAmxTpl(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...) {}
    // public void LogLocFmtTpl(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...) {}

    public void LogStackTrace(LogLevel lvl, const char[] msg) {}
    public void LogStackTraceAmxTpl(LogLevel lvl, const char[] fmt, any ...) {}
    // public void LogStackTraceFmtTpl(LogLevel lvl, const char[] fmt, any ...) {}

    public void ThrowError(LogLevel lvl, const char[] msg) {}
    public void ThrowErrorAmxTpl(LogLevel lvl, const char[] fmt, any ...) {}
    // public void ThrowErrorFmtTpl(LogLevel lvl, const char[] fmt, any ...) {}

    public void Trace(const char[] msg) {}
    public void TraceAmxTpl(const char[] fmt, any ...) {}
    // public void TraceFmtTpl(const char[] fmt, any ...) {}

    public void Debug(const char[] msg) {}
    public void DebugAmxTpl(const char[] fmt, any ...) {}
    // public void DebugFmtTpl(const char[] fmt, any ...) {}

    public void Info(const char[] msg) {}
    public void InfoAmxTpl(const char[] fmt, any ...) {}
    // public void InfoFmtTpl(const char[] fmt, any ...) {}

    public void Warn(const char[] msg) {}
    public void WarnAmxTpl(const char[] fmt, any ...) {}
    // public void WarnFmtTpl(const char[] fmt, any ...) {}

    public void Error(const char[] msg) {}
    public void ErrorAmxTpl(const char[] fmt, any ...) {}
    // public void ErrorFmtTpl(const char[] fmt, any ...) {}

    public void Fatal(const char[] msg) {}
    public void FatalAmxTpl(const char[] fmt, any ...) {}
    // public void FatalFmtTpl(const char[] fmt, any ...) {}

    public void Flush() {}
    public LogLevel GetFlushLevel() { return LogLevel_Off; }
    public void FlushOn(LogLevel lvl) {}

    public bool ShouldBacktrace() { return false; }
    public void EnableBacktrace(int num) {}
    public void DisableBacktrace() {}
    public void DumpBacktrace() {}

    #pragma deprecated Due to multi-threaded safety issues, expected to be removed in the next version
    public void AddSink(Sink sink) {}
    #pragma deprecated Due to multi-threaded safety issues, expected to be removed in the next version
    public bool DropSink(Sink sink) { return false; }

    public void SetErrorHandler(Log4spErrorCallback callback) {}
}

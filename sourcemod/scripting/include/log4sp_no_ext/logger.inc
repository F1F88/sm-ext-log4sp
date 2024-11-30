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

static char __message[2048];

methodmap Logger < Handle
{
    public Logger(const char[] name, Sink[] sinks, int numSinks, bool async=false, AsyncOverflowPolicy policy=AsyncOverflowPolicy_Block) { return view_as<Logger>(INVALID_HANDLE); }

    public static Logger Get(const char[] name) { return view_as<Logger>(INVALID_HANDLE); }

    public static Logger CreateServerConsoleLogger(const char[] name, bool async=false, AsyncOverflowPolicy policy=AsyncOverflowPolicy_Block) { return view_as<Logger>(INVALID_HANDLE); }

    public static Logger CreateBaseFileLogger(const char[] name, const char[] file, bool truncate=false, bool async=false, AsyncOverflowPolicy policy=AsyncOverflowPolicy_Block) { return view_as<Logger>(INVALID_HANDLE); }

    public static Logger CreateRotatingFileLogger(const char[] name, const char[] file, int maxFileSize, int maxFiles, bool rotateOnOpen=false, bool async=false, AsyncOverflowPolicy policy=AsyncOverflowPolicy_Block) { return view_as<Logger>(INVALID_HANDLE); }

    public static Logger CreateDailyFileLogger(const char[] name, const char[] file, int hour=0, int minute=0, bool truncate=false, int maxFiles=0, bool async=false, AsyncOverflowPolicy policy=AsyncOverflowPolicy_Block) { return view_as<Logger>(INVALID_HANDLE); }

    public void GetName(char[] buffer, int maxlen) {}

    public LogLevel GetLevel() {
        return LogLevel_Info;
    }

    public void SetLevel(LogLevel lvl) {}

    public void SetPattern(const char[] pattern, PatternTimeType type=PatternTimeType_Local) {}

    public bool ShouldLog(LogLevel lvl) {
        return lvl >= this.GetLevel() && lvl < LogLevel_Off;
    }

    public void Log(LogLevel lvl, const char[] msg) {
        switch (lvl) {
            case LogLevel_Info:     LogMessage("[%s] %s", LOG4SP_LEVEL_NAME_INFO, msg);
            case LogLevel_Warn:     LogMessage("[%s] %s", LOG4SP_LEVEL_NAME_WARN, msg);
            case LogLevel_Error:    LogError("[%s] %s", LOG4SP_LEVEL_NAME_ERROR, msg);
            case LogLevel_Fatal:    LogError("[%s] %s", LOG4SP_LEVEL_NAME_FATAL, msg);
        }
    }
    public void LogAmxTpl(LogLevel lvl, const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 4);
        switch (lvl) {
            case LogLevel_Info:     LogMessage("[%s] %s", LOG4SP_LEVEL_NAME_INFO, __message);
            case LogLevel_Warn:     LogMessage("[%s] %s", LOG4SP_LEVEL_NAME_WARN, __message);
            case LogLevel_Error:    LogError("[%s] %s", LOG4SP_LEVEL_NAME_ERROR, __message);
            case LogLevel_Fatal:    LogError("[%s] %s", LOG4SP_LEVEL_NAME_FATAL, __message);
        }
    }
    // public void LogFmtTpl(LogLevel lvl, const char[] fmt, any ...) {}

    public void LogSrc(LogLevel lvl, const char[] msg) {
        switch (lvl) {
            case LogLevel_Info:     LogMessage("[%s] %s", LOG4SP_LEVEL_NAME_INFO, msg);
            case LogLevel_Warn:     LogMessage("[%s] %s", LOG4SP_LEVEL_NAME_WARN, msg);
            case LogLevel_Error:    LogError("[%s] %s", LOG4SP_LEVEL_NAME_ERROR, msg);
            case LogLevel_Fatal:    LogError("[%s] %s", LOG4SP_LEVEL_NAME_FATAL, msg);
        }
    }
    public void LogSrcAmxTpl(LogLevel lvl, const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 4);
        switch (lvl) {
            case LogLevel_Info:     LogMessage("[%s] %s", LOG4SP_LEVEL_NAME_INFO, __message);
            case LogLevel_Warn:     LogMessage("[%s] %s", LOG4SP_LEVEL_NAME_WARN, __message);
            case LogLevel_Error:    LogError("[%s] %s", LOG4SP_LEVEL_NAME_ERROR, __message);
            case LogLevel_Fatal:    LogError("[%s] %s", LOG4SP_LEVEL_NAME_FATAL, __message);
        }
    }
    // public void LogSrcFmtTpl(LogLevel lvl, const char[] fmt, any ...) {}

    public void LogLoc(const char[] file, int line, const char[] func, LogLevel lvl, const char[] msg) {
        switch (lvl) {
            case LogLevel_Info:     LogMessage("[%s] %s", LOG4SP_LEVEL_NAME_INFO, msg);
            case LogLevel_Warn:     LogMessage("[%s] %s", LOG4SP_LEVEL_NAME_WARN, msg);
            case LogLevel_Error:    LogError("[%s] %s", LOG4SP_LEVEL_NAME_ERROR, msg);
            case LogLevel_Fatal:    LogError("[%s] %s", LOG4SP_LEVEL_NAME_FATAL, msg);
        }
    }
    public void LogLocAmxTpl(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 7);
        switch (lvl) {
            case LogLevel_Info:     LogMessage("[%s] %s", LOG4SP_LEVEL_NAME_INFO, __message);
            case LogLevel_Warn:     LogMessage("[%s] %s", LOG4SP_LEVEL_NAME_WARN, __message);
            case LogLevel_Error:    LogError("[%s] %s", LOG4SP_LEVEL_NAME_ERROR, __message);
            case LogLevel_Fatal:    LogError("[%s] %s", LOG4SP_LEVEL_NAME_FATAL, __message);
        }
    }
    // public void LogLocFmtTpl(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...) {}

    public void LogStackTrace(LogLevel lvl, const char[] msg) {
        LogStackTrace(msg);
    }
    public void LogStackTraceAmxTpl(LogLevel lvl, const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 4);
        LogStackTrace(__message);
    }
    // public void LogStackTraceFmtTpl(LogLevel lvl, const char[] fmt, any ...) {}

    public void ThrowError(LogLevel lvl, const char[] msg) {
        ThrowError(msg);
    }
    public void ThrowErrorAmxTpl(LogLevel lvl, const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 4);
        ThrowError(__message);
    }
    // public void ThrowErrorFmtTpl(LogLevel lvl, const char[] fmt, any ...) {}

    public void Trace(const char[] msg) {}
    public void TraceAmxTpl(const char[] fmt, any ...) {}
    // public void TraceFmtTpl(const char[] fmt, any ...) {}

    public void Debug(const char[] msg) {}
    public void DebugAmxTpl(const char[] fmt, any ...) {}
    // public void DebugFmtTpl(const char[] fmt, any ...) {}

    public void Info(const char[] msg) {
        LogMessage("[%s] %s", LOG4SP_LEVEL_NAME_INFO, msg);
    }
    public void InfoAmxTpl(const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 3);
        LogMessage("[%s] %s", LOG4SP_LEVEL_NAME_INFO, __message);
    }
    // public void InfoFmtTpl(const char[] fmt, any ...) {}

    public void Warn(const char[] msg) {
        LogMessage("[%s] %s", LOG4SP_LEVEL_NAME_WARN, msg);
    }
    public void WarnAmxTpl(const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 3);
        LogMessage("[%s] %s", LOG4SP_LEVEL_NAME_WARN, __message);
    }
    // public void WarnFmtTpl(const char[] fmt, any ...) {}

    public void Error(const char[] msg) {
        LogError("[%s] %s", LOG4SP_LEVEL_NAME_ERROR, msg);
    }
    public void ErrorAmxTpl(const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 3);
        LogError("[%s] %s", LOG4SP_LEVEL_NAME_ERROR, __message);
    }
    // public void ErrorFmtTpl(const char[] fmt, any ...) {}

    public void Fatal(const char[] msg) {
        LogError("[%s] %s", LOG4SP_LEVEL_NAME_FATAL, msg);
    }
    public void FatalAmxTpl(const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 3);
        LogError("[%s] %s", LOG4SP_LEVEL_NAME_FATAL, __message);
    }
    // public void FatalFmtTpl(const char[] fmt, any ...) {}

    public void Flush() {}
    public LogLevel GetFlushLevel() { return LogLevel_Off; }
    public void FlushOn(LogLevel lvl) {}

    public bool ShouldBacktrace() { return false; }
    public void EnableBacktrace(int num) {}
    public void DisableBacktrace() {}
    public void DumpBacktrace() {}

    public void AddSink(Sink sink) {}
    public bool DropSink(Sink sink) { return false; }

    public void SetErrorHandler(Log4spErrorCallback callback) {}
}

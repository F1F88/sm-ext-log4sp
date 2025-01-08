#if defined _log_for_sourcepawn_logger_included
 #endinput
#endif
#define _log_for_sourcepawn_logger_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp_no_ext/common>
#include <log4sp_no_ext/sinks/sink>


typeset LoggerErrorHandler
{
    function void (const char[] msg);
};

typeset LoggerApplyAllCallback
{
    function void (Logger logger);
    function void (Logger logger, any data);
};

static char __message[2048];

methodmap Logger < Handle
{
    public Logger(const char[] name, Sink[] sinks, int numSinks, bool async = false, AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block) {
        return view_as<Logger>(INVALID_HANDLE);
    }

    public static Logger Get(const char[] name) {
        return view_as<Logger>(INVALID_HANDLE);
    }

    public static void ApplyAll(LoggerApplyAllCallback callback, any data = 0) {}

    public static Logger CreateServerConsoleLogger(const char[] name, bool async = false, AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block) {
        return view_as<Logger>(INVALID_HANDLE);
    }

    public static Logger CreateBaseFileLogger(const char[] name, const char[] file, bool truncate = false, bool async = false, AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block) {
        return view_as<Logger>(INVALID_HANDLE);
    }

    public static Logger CreateClientChatLogger(const char[] name, bool async = false, AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block) {
        return view_as<Logger>(INVALID_HANDLE);
    }

    public static Logger CreateClientConsoleLogger(const char[] name, bool async = false, AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block) {
        return view_as<Logger>(INVALID_HANDLE);
    }

    public static Logger CreateRotatingFileLogger(const char[] name, const char[] file, int maxFileSize, int maxFiles, bool rotateOnOpen = false, bool async = false, AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block) {
        return view_as<Logger>(INVALID_HANDLE);
    }

    public static Logger CreateDailyFileLogger(const char[] name, const char[] file, int hour = 0, int minute = 0, bool truncate = false, int maxFiles = 0, bool async = false, AsyncOverflowPolicy policy = AsyncOverflowPolicy_Block) {
        return view_as<Logger>(INVALID_HANDLE);
    }

    public int GetName(char[] buffer, int maxlen) {
        return 0;
    }

    public int GetNameLength() {
        return 0;
    }

    public LogLevel GetLevel() {
        return LogLevel_Info;
    }

    public void SetLevel(LogLevel lvl) {}

    public void SetPattern(const char[] pattern, PatternTimeType type = PatternTimeType_Local) {}

    public bool ShouldLog(LogLevel lvl) {
        return lvl >= this.GetLevel() && lvl < LogLevel_Off;
    }

    public void Log(LogLevel lvl, const char[] msg) {
        __Log(lvl, msg);
    }
    public void LogEx(LogLevel lvl, const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 4);
        __Log(lvl, __message);
    }
    public void LogAmxTpl(LogLevel lvl, const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 4);
        __Log(lvl, __message);
    }

    public void LogSrc(LogLevel lvl, const char[] msg) {
        __LogSrc(lvl, msg);
    }
    public void LogSrcEx(LogLevel lvl, const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 4);
        __LogSrc(lvl, __message);
    }
    public void LogSrcAmxTpl(LogLevel lvl, const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 4);
        __LogSrc(lvl, __message);
    }

    public void LogLoc(const char[] file, int line, const char[] func, LogLevel lvl, const char[] msg) {
        __LogLoc(file, line, lvl, msg);
    }
    public void LogLocEx(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 7);
        __LogLoc(file, line, lvl, __message);
    }
    public void LogLocAmxTpl(const char[] file, int line, const char[] func, LogLevel lvl, const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 7);
        __LogLoc(file, line, lvl, __message);
    }

    public void LogStackTrace(LogLevel lvl, const char[] msg) {
        __LogStackTrace(lvl, msg);
    }
    public void LogStackTraceEx(LogLevel lvl, const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 4);
        __LogStackTrace(lvl, __message);
    }
    public void LogStackTraceAmxTpl(LogLevel lvl, const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 4);
        __LogStackTrace(lvl, __message);
    }

    public void ThrowError(LogLevel lvl, const char[] msg) {
        __ThrowError(lvl, msg);
    }
    public void ThrowErrorEx(LogLevel lvl, const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 4);
        __ThrowError(lvl, __message);
    }
    public void ThrowErrorAmxTpl(LogLevel lvl, const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 4);
        __ThrowError(lvl, __message);
    }

    public void Trace(const char[] msg) {}
    public void TraceEx(const char[] fmt, any ...) {}
    public void TraceAmxTpl(const char[] fmt, any ...) {}

    public void Debug(const char[] msg) {}
    public void DebugEx(const char[] fmt, any ...) {}
    public void DebugAmxTpl(const char[] fmt, any ...) {}

    public void Info(const char[] msg) {
        __Log(LogLevel_Info, msg);
    }
    public void InfoEx(const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 3);
        __Log(LogLevel_Info, __message);
    }
    public void InfoAmxTpl(const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 3);
        __Log(LogLevel_Info, __message);
    }

    public void Warn(const char[] msg) {
        __Log(LogLevel_Warn, msg);
    }
    public void WarnEx(const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 3);
        __Log(LogLevel_Warn, __message);
    }
    public void WarnAmxTpl(const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 3);
        __Log(LogLevel_Warn, __message);
    }

    public void Error(const char[] msg) {
        __Log(LogLevel_Error, msg);
    }
    public void ErrorEx(const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 3);
        __Log(LogLevel_Error, __message);
    }
    public void ErrorAmxTpl(const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 3);
        __Log(LogLevel_Error, __message);
    }

    public void Fatal(const char[] msg) {
        __Log(LogLevel_Fatal, msg);
    }
    public void FatalEx(const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 3);
        __Log(LogLevel_Fatal, __message);
    }
    public void FatalAmxTpl(const char[] fmt, any ...) {
        VFormat(__message, sizeof(__message), fmt, 3);
        __Log(LogLevel_Fatal, __message);
    }

    public void Flush() {}
    public LogLevel GetFlushLevel() {
        return LogLevel_Off;
    }
    public void FlushOn(LogLevel lvl) {}

    public bool ShouldBacktrace() {
        return false;
    }
    public void EnableBacktrace(int num) {}
    public void DisableBacktrace() {}
    public void DumpBacktrace() {}

    public void AddSink(Sink sink) {}
    public void DropSink(Sink sink) {}

    public void SetErrorHandler(LoggerErrorHandler handler) {}
}


static void __Log(LogLevel lvl, const char[] msg) {
    switch (lvl) {
        case LogLevel_Info:     LogMessage("["... LOG4SP_LEVEL_NAME_INFO ..."] %s", msg);
        case LogLevel_Warn:     LogMessage("["... LOG4SP_LEVEL_NAME_WARN ..."] %s", msg);
        case LogLevel_Error:    LogError("["... LOG4SP_LEVEL_NAME_ERROR ..."] %s", msg);
        case LogLevel_Fatal:    LogError("["... LOG4SP_LEVEL_NAME_FATAL ..."] %s", msg);
        case LogLevel_Off:      LogError("["... LOG4SP_LEVEL_NAME_OFF ..."] %s", msg);
    }
}

static void __LogSrc(LogLevel lvl, const char[] msg) {
    FrameIterator iter = new FrameIterator();

    iter.Next();
    iter.Next();
    iter.Next();

    char file[PLATFORM_MAX_PATH];
    iter.GetFilePath(file, sizeof(file));
    int line = iter.LineNumber;

    delete iter;

    __LogLoc(file, line, lvl, msg);
}

static void __LogLoc(const char[] file, int line, LogLevel lvl, const char[] msg) {
    int sep = FindCharInString(file, '\\', true) + 1;
    sep = sep != 0 ? sep : FindCharInString(file, '/', true) + 1;
    Format(__message, sizeof(__message), "[%s:%d] %s", file[sep], line, msg);
    __Log(lvl, __message);
}

static void __LogStackTrace(LogLevel lvl, const char[] msg) {
    switch (lvl) {
        case LogLevel_Info:     LogStackTrace("["... LOG4SP_LEVEL_NAME_INFO ..."] %s", msg);
        case LogLevel_Warn:     LogStackTrace("["... LOG4SP_LEVEL_NAME_WARN ..."] %s", msg);
        case LogLevel_Error:    LogStackTrace("["... LOG4SP_LEVEL_NAME_ERROR ..."] %s", msg);
        case LogLevel_Fatal:    LogStackTrace("["... LOG4SP_LEVEL_NAME_FATAL ..."] %s", msg);
        case LogLevel_Off:      LogStackTrace("["... LOG4SP_LEVEL_NAME_OFF ..."] %s", msg);
    }
}

static void __ThrowError(LogLevel lvl, const char[] msg) {
    switch (lvl) {
        case LogLevel_Info:     ThrowError("["... LOG4SP_LEVEL_NAME_INFO ..."] %s", msg);
        case LogLevel_Warn:     ThrowError("["... LOG4SP_LEVEL_NAME_WARN ..."] %s", msg);
        case LogLevel_Error:    ThrowError("["... LOG4SP_LEVEL_NAME_ERROR ..."] %s", msg);
        case LogLevel_Fatal:    ThrowError("["... LOG4SP_LEVEL_NAME_FATAL ..."] %s", msg);
        case LogLevel_Off:      ThrowError("["... LOG4SP_LEVEL_NAME_OFF ..."] %s", msg);
    }
}

#include <sourcemod>
#include <testing>

#include <log4sp>

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_NAME             "Log4sp Test2"
#define PLUGIN_AUTHOR           "F1F88"
#define PLUGIN_VERSION          LOG4SP_EXT_VERSION
#define PLUGIN_DESCRIPTION      "Log for SourcePawn test2"
#define PLUGIN_URL              "https://github.com/F1F88/sm-ext-log4sp"

public Plugin myinfo = {
    name        = PLUGIN_NAME,
    author      = PLUGIN_AUTHOR,
    description = PLUGIN_DESCRIPTION,
    version     = PLUGIN_VERSION,
    url         = PLUGIN_URL
};


public void OnPluginStart()
{
    PrintToServer("****************** Log4sp Test ******************");

    LoadTranslations("common.phrases");

    RegConsoleCmd("sm_log4sp_test2", CB_CMD);
}

Action CB_CMD(int client, int args)
{
    SetTestContext("Log4sp Test");

    Test_DefaultLogger();

    Test_ServerConsoleLogger();

    Test_RotatingFileLogger();

    Test_DailyFileLogger();

    Test_ClientConsoleAllLogger();

    Test_ClientChatAllLogger();

    Test_BaseFileLogger();

    return Plugin_Handled;
}

void Test_DefaultLogger()
{
    SetTestContext("Log4sp Test default logger");

    Logger log = Logger.Get(LOG4SP_DEFAULT_LOGGER_NAME);
    if (log == INVALID_HANDLE)
    {
        ThrowError("Failed to get default logger. (1)");
    }

    Test_Logger(log, LOG4SP_DEFAULT_LOGGER_NAME);

    delete log;
    log = Logger.Get(LOG4SP_DEFAULT_LOGGER_NAME);
    if (log == INVALID_HANDLE)
    {
        ThrowError("Failed to get default logger. (2)");
    }
}

void Test_ServerConsoleLogger()
{
    SetTestContext("Log4sp Test server console logger");

    Logger log;

    log = Logger.CreateServerConsoleLogger("test-server-console-logger");
    Test_Logger(log, "test-server-console-logger");
    delete log;
}

void Test_RotatingFileLogger()
{
    SetTestContext("Log4sp Test rotating file logger");

    Logger log;

    log = Logger.Get("test-rotate-file-logger");
    if (log != INVALID_HANDLE)
        delete log;

    log = Logger.CreateRotatingFileLogger("test-rotate-file-logger", "logs/test/rotate.log", 1024 * 1024, 3);
    Test_Logger(log, "test-rotate-file-logger");
    delete log;
}

void Test_DailyFileLogger()
{
    SetTestContext("Log4sp Test daily file logger");

    Logger log;

    log = Logger.Get("test-daily-file-logger");
    if (log != INVALID_HANDLE)
        delete log;

    log = Logger.CreateDailyFileLogger("test-daily-file-logger", "logs/test/daily.log", .truncate=true);
    Test_Logger(log, "test-daily-file-logger");
    delete log;
}

void Test_ClientConsoleAllLogger()
{
    SetTestContext("Log4sp Test client console all logger");

    Logger log;

    log = Logger.Get("test-client-console-all-logger");
    if (log != INVALID_HANDLE)
        delete log;

    Sink sinks[1];
    sinks[0] = new ClientConsoleAllSink();

    log = new Logger("test-client-console-all-logger", sinks, sizeof(sinks));

    delete sinks[0];

    Test_Logger(log, "test-client-console-all-logger");
    delete log;
}

void Test_ClientChatAllLogger()
{
    SetTestContext("Log4sp Test client chat all logger");

    Logger log;

    log = Logger.Get("test-client-chat-all-logger");
    if (log != INVALID_HANDLE)
        delete log;

    Sink sinks[1];
    sinks[0] = new ClientChatAllSink();

    log = new Logger("test-client-chat-all-logger", sinks, sizeof(sinks));

    delete sinks[0];

    Test_Logger(log, "test-client-chat-all-logger");
    delete log;
}

void Test_BaseFileLogger()
{
    SetTestContext("Log4sp Test base file logger");

    Logger log;

    log = Logger.Get("test-base-file-logger");
    if (log != INVALID_HANDLE)
        delete log;

    log = Logger.CreateBaseFileLogger("test-base-file-logger", "logs/test/base.log", .truncate=true);
    Test_Logger(log, "test-base-file-logger");
    delete log;
}

void Test_Logger(Logger logger, const char[] name)
{
    Test_Logger_Log(logger);
    Test_Logger_Member(logger, name);
    Test_Logger_Backtrace(logger);
    Test_Logger_Update_Sinks(logger);
    Test_Logger_Error_Handler(logger);
}

/**
 * 仅测试输出，不改变 logger 任何属性
 */
void Test_Logger_Log(Logger logger)
{
    PrintToServer("PrintToServer() | d: %d | u: %u | b: %b | f: %f | x: %x | X: %X |", -1, -1, -1, -1.0, -1, -1);
    PrintToServer("PrintToServer() | s: %s | t: %t | T: %T | c: %c | L: %L | N: %N |", "string", "Yes", "No", LANG_SERVER, 's', 0, 0);

    // Log, LogEx
    logger.Log(LogLevel_Fatal, "Log()");
    logger.LogEx(LogLevel_Fatal, "LogEx() | d: %d | u: %u | b: %b | f: %f | x: %x | X: %X |", -1, -1, -1, -1.0, -1, -1);

    // LogSrc, LogSrcEx
    logger.LogSrc(LogLevel_Error, "LogSrc()");
    logger.LogSrcEx(LogLevel_Error, "LogSrcEx() | s: %s | t: %t | T: %T | c: %c | L: %L | N: %N |", "string", "Yes", "No", LANG_SERVER, 's', 0, 0);

    // LogLoc, LogLocEx
    logger.LogLoc("loc-file.log", 9527, "loc_function", LogLevel_Warn, "LogLoc()");
    logger.LogLocEx("loc-file.log", 9527, "loc_function", LogLevel_Warn, "LogLocEx() | d: %d | u: %u | b: %b | f: %f | x: %x | X: %X |", -1, -1, -1, -1.0, -1, -1);

    // LogStackTrace, LogStackTraceEx
    logger.LogStackTrace(LogLevel_Info, "LogStackTrace()");
    logger.LogStackTraceEx(LogLevel_Info, "LogStackTraceEx() | s: %s | t: %t | T: %T | c: %c | L: %L | N: %N |", "string", "Yes", "No", LANG_SERVER, 's', 0, 0);

    // ThrowError, ThrowErrorEx

    // Trace, TraceEx
    logger.Trace("Trace()");
    logger.TraceEx("TraceEx() | d: %d | u: %u | b: %b | f: %f | x: %x | X: %X |", -1, -1, -1, -1.0, -1, -1);

    // Debug, DebugEx
    logger.Debug("LogSrc()");
    logger.DebugEx("LogSrcEx() | s: %s | t: %t | T: %T | c: %c | L: %L | N: %N |", "string", "Yes", "No", LANG_SERVER, 's', 0, 0);

    // Info, InfoEx
    logger.Info("Info()");
    logger.InfoEx("InfoEx() | d: %d | u: %u | b: %b | f: %f | x: %x | X: %X |", -1, -1, -1, -1.0, -1, -1);

    // Warn, WarnEx
    logger.Warn("Warn()");
    logger.WarnEx("WarnEx() | s: %s | t: %t | T: %T | c: %c | L: %L | N: %N |", "string", "Yes", "No", LANG_SERVER, 's', 0, 0);

    // Error, ErrorEx
    logger.Error("Error()");
    logger.ErrorEx("ErrorEx() | d: %d | u: %u | b: %b | f: %f | x: %x | X: %X |", -1, -1, -1, -1.0, -1, -1);

    // Fatal, FatalEx
    logger.Fatal("Fatal()");
    logger.FatalEx("FatalEx() | s: %s | t: %t | T: %T | c: %c | L: %L | N: %N |", "string", "Yes", "No", LANG_SERVER, 's', 0, 0);
}

/**
 * 传入的必须是创建后没有修改任何属性的 logger
 * 如果你修改了 level 或者其他任何属性，可能导致测试失败
 */
void Test_Logger_Member(Logger logger, const char[] name)
{
    // GetName
    char buffer[64];
    logger.GetName(buffer, sizeof(buffer));
    AssertStrEq("GetName()", buffer, name);

    // For restore
    LogLevel oriLogLvl = logger.GetLevel();

    // SetLeve
    logger.SetLevel(LogLevel_Debug);
    AssertEq("SetLevel(debug)", logger.GetLevel(), LogLevel_Debug);

    // ShouldLog
    AssertTrue("ShouldLog(info)", logger.ShouldLog(LogLevel_Info));
    AssertFalse("ShouldLog(trace)", logger.ShouldLog(LogLevel_Trace));

    // GetLevel() and Restore log level
    logger.SetLevel(oriLogLvl);
    AssertEq("GetLevel()", logger.GetLevel(), oriLogLvl);

    // Flush
    logger.Flush();

    // For restore
    LogLevel oriFlushLvl = logger.GetFlushLevel();

    // FlushOn
    logger.FlushOn(LogLevel_Info);
    AssertEq("FlushOn(info)", logger.GetFlushLevel(), LogLevel_Info);

    // GetFlushLevel
    AssertEq("GetFlushLevel()", logger.GetFlushLevel(), LogLevel_Info);

    // Restore flush level
    logger.FlushOn(oriFlushLvl);

    // ShouldBacktrace
    AssertFalse("ShouldBacktrace()", logger.ShouldBacktrace());

    // EnableBacktrace
    logger.EnableBacktrace(32);
    AssertTrue("EnableBacktrace(32)", logger.ShouldBacktrace());

    // DisableBacktrace
    logger.DisableBacktrace();
    AssertFalse("DisableBacktrace()", logger.ShouldBacktrace());
}

void Test_Logger_Backtrace(Logger logger)
{
    // 传入的 logger 不应该开启 backtrace
    AssertFalse("Init check - ShouldBacktrace()", logger.ShouldBacktrace());

    // 保存原来的属性值以在测试结束后恢复
    LogLevel originLogLevel = logger.GetLevel();

    // 初始化
    logger.DisableBacktrace();
    AssertFalse("DisableBacktrace()", logger.ShouldBacktrace());

    logger.SetLevel(LogLevel_Info);
    AssertEq("SetLevel(info)", logger.GetLevel(), LogLevel_Info);

    logger.EnableBacktrace(2);
    AssertTrue("EnableBacktrace(2)", logger.ShouldBacktrace());

    // 此处的 trace 和 debug 应该被忽略
    logger.Trace("Trace() | 1 | not log");
    logger.Debug("Debug() | 2 | not log");
    logger.Trace("Trace() | 3 | log");
    logger.Debug("Debug() | 4 | log");

    // dump 3, 4 日志
    logger.DumpBacktrace();

    // 结束测试
    logger.DisableBacktrace();
    AssertFalse("DisableBacktrace()", logger.ShouldBacktrace());

    // 恢复属性值
    logger.SetLevel(originLogLevel);
    AssertEq("SetLevel(origin)", logger.GetLevel(), originLogLevel);
}

void Test_Logger_Update_Sinks(Logger logger)
{
    Sink sink = new BaseFileSink("addons/sourcemod/logs/log4sp-test-logger-sinks.log");
    logger.AddSink(sink);
    Test_Logger_Log(logger);
    logger.DropSink(sink);
    delete sink;
}

void Test_Logger_Error_Handler(Logger logger)
{
    logger.FatalEx("FatalEx() 1 | Param: %s | But no arg pass.");
    logger.LogSrcEx(LogLevel_Fatal, "LogSrcEx(Fatal) 2 | Param: %s %s | But no arg pass.");

    logger.SetErrorHandler(ErrorHandler);

    logger.FatalEx("FatalEx() 3 | Param: %s %s %s | But no arg pass.");
    logger.LogSrcEx(LogLevel_Fatal, "FatalEx() 4 | Param: %s %s %s %s| But no arg pass.");
}


void ErrorHandler(const char[] msg)
{
    static int cnt = 0;
    PrintToServer("custom error handler: %d | msg=%s |", ++cnt, msg);
}

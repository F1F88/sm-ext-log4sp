#include <sourcemod>
#include <testing>

#include <log4sp>

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_NAME             "Log4sp Test"
#define PLUGIN_AUTHOR           "F1F88"
#define PLUGIN_VERSION          LOG4SP_EXT_VERSION
#define PLUGIN_DESCRIPTION      "Logging for SourcePawn test"
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


    benchFmt1__(Logger.Get(LOG4SP_DEFAULT_LOGGER_NAME), 0);

    // Test_DefaultLogger();

    // Test_ServerConsoleLogger();

    return Plugin_Handled;
}

void benchFmt1__(Logger log, int client)
{
    log.InfoEx("--- 测试 char 效果 %% %% |%c| |%-c| |%7c| |%.3c| |%-7c| |%07c| |%-07c|---", 'a', 'b', 'c', 'D', 'E', 'F', 'G');

    // PrintToServer("--- 测试 bin 效果   |%b| |%9b| |%09b| |%-9b| |%-09b|---", 23, 23, 23, 23, 23);
    // PrintToServer("--- 测试 bin 效果   |%b| |%9b| |%09b| |%-9b| |%-09b|---", -8192, -8192, -8192, -8192, -8192);

    log.InfoEx("--- 测试 bin 效果   |%b| |%9b| |%09b| |%-9b| |%-09b|---", 23, 23, 23, 23, 23);
    log.InfoEx("--- 测试 bin 效果   |%b| |%9b| |%09b| |%-9b| |%-09b|---", -8192, -8192, -8192, -8192, -8192);
    log.InfoEx("--- 测试 int-d 效果 |%d| |%9d| |%09d| |%-9d| |%-09d|---", 79247, 79247, 79247, 79247, 79247);
    log.InfoEx("--- 测试 int-i 效果 |%i| |%9i| |%09i| |%-9i| |%-09i|---", -1234, -1234, -1234, -1234, -1234);
    log.InfoEx("--- 测试 int-u 效果 |%u| |%9u| |%09u| |%-9u| |%-09u|---", 1<<31, 1<<31, 1<<31, 1<<31, 1<<31);
    log.InfoEx("--- 测试 int-X 效果 |%X| |%9X| |%09X| |%-9X| |%-09X|---", 61944, 61944, 61944, 61944, 61944);
    log.InfoEx("--- 测试 int-x 效果 |%x| |%9x| |%09x| |%-9x| |%-09x|---", -3871, -3871, -3871, -3871, -3871);
    log.InfoEx("--- 测试 float 效果 |%f| |%9f| |%09f| |%-9f| |%-09f| |%09.2f| |%-09.2f|---", FLOAT_PI, FLOAT_PI, FLOAT_PI, FLOAT_PI, FLOAT_PI, FLOAT_PI, FLOAT_PI);
    log.InfoEx("--- 测试 float 效果 |%f| |%20f| |%020f| |%-9f| |%-09f| |%09.2f| |%-09.2f|---", -4.0, -4.0, -4.0, -4.0, -4.0, -4.0, -4.0);

    log.InfoEx("--- 测试 L 效果 |%L| |%9L| |%09L| |%-9L| |%-09L| |%09.2L| |%-09.2L|---", client, client, client, client, client, client, client);

    log.InfoEx("--- 测试 N 效果 |%N| |%9N| |%09N| |%-9N| |%-09N| |%09.2N| |%-09.2N|---", client, client, client, client, client, client, client);

    log.InfoEx("--- 测试 s 效果 |%s| |%9s| |%09s| |%-9s| |%-09s| |%09.2s| |%-09.2s|---", "str-111", "str-222", "str-333", "str-444", "str-555", "str-666", "str-777");

    log.InfoEx("ttt - %t", "Changing map", "nmo_chinatown");
    log.InfoEx("TTT - %T", "Changing map", LANG_SERVER, "nmo_chinatown");
}

void Test_DefaultLogger()
{
    SetTestContext("Log4sp Test default logger");

    Logger log = Logger.Get(LOG4SP_DEFAULT_LOGGER_NAME);
    if (log == INVALID_HANDLE)
    {
        ThrowError("Failed to get default logger. (1)");
    }

    Test_Logger(log, LOG4SP_DEFAULT_LOGGER_NAME, false);

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

    log = Logger.Get("test-sync-server-console-logger");
    if (log != INVALID_HANDLE)
        delete log;

    log = Logger.Get("test-async-server-console-logger");
    if (log != INVALID_HANDLE)
        delete log;

    log = Logger.CreateServerConsoleLogger("test-sync-server-console-logger");
    Test_Logger(log, "test-sync-server-console-logger", false);
    delete log;

    log = Logger.CreateServerConsoleLogger("test-async-server-console-logger");
    Test_Logger(log, "test-async-server-console-logger", true);
    delete log;
}

void Test_Logger(Logger logger, const char[] name, bool async)
{
    Test_Logger_Log(logger);
    Test_Logger_Member(logger, name);
    Test_Logger_Backtrace(logger);
    Test_Logger_Update_Sinks(logger, async);
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

void Test_Logger_Update_Sinks(Logger logger, bool mt)
{
    Sink sink = new BaseFileSink("addons/sourcemod/logs/log4sp-test-logger-sinks.log", true, mt);
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


void ErrorHandler(Logger logger, const char[] msg)
{
    static int cnt = 0;
    PrintToServer("detect an error: %d | hdl=%s | msg=%s |", ++cnt, logger, msg);
}

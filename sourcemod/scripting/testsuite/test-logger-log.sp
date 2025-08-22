#pragma semicolon 1
#pragma newdecls required

/**
 * Fix: "Not enough space on the heap"
 * 似乎是由于 TestSink.DrainLastLine 的调用堆栈过深
 */
#pragma dynamic 131072

#include <sourcemod>
#include <log4sp>

#include "test_sink"
#include "test_utils"


#define LOGGER_NAME             "test-log"


public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_log", Command_Test);
}

Action Command_Test(int args)
{
    PrintToServer("---- START TEST LOG ----");

    PrepareTestPath("logger-log/");

    TestLog();

    TestLogSrc();

    TestLogLoc();

    TestLogStackTrace();

    PrintToServer("---- STOP TEST LOG ----");
    return Plugin_Handled;
}


void TestLog()
{
    SetTestContext("Test Logger Log");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.Log(LogLevel_Info, "test message 1");
    AssertStrMatch("Log line match", sink.DrainLastLine(), P_PREFIX ... "test message 1");

    logger.LogEx(LogLevel_Info, "test message %d", 2);
    AssertStrMatch("LogEx line match", sink.DrainLastLine(), P_PREFIX ... "test message 2");

    logger.LogAmxTpl(LogLevel_Info, "test message %d", 3);
    AssertStrMatch("LogAmxTpl line match", sink.DrainLastLine(), P_PREFIX ... "test message 3");

    logger.Close();
    sink.Close();
}


void TestLogSrc()
{
    SetTestContext("Test Logger LogSrc");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.LogSrc(LogLevel_Info, "test message 1");
    AssertStrMatch("LogSrc line match", sink.DrainLastLine(), P_PREFIX ... "\\[test-logger-log.sp:[0-9]+\\] test message 1");

    logger.LogSrcEx(LogLevel_Info, "test message %d", 2);
    AssertStrMatch("LogSrcEx line match", sink.DrainLastLine(), P_PREFIX ... "\\[test-logger-log.sp:[0-9]+\\] test message 2");

    logger.LogSrcAmxTpl(LogLevel_Info, "test message %d", 3);
    AssertStrMatch("LogSrcAmxTpl line match", sink.DrainLastLine(), P_PREFIX ... "\\[test-logger-log.sp:[0-9]+\\] test message 3");

    logger.Close();
    sink.Close();
}


void TestLogLoc()
{
    SourceLoc locLinux;
    strcopy(locLinux.filename, sizeof(SourceLoc::filename), "/home/sm-ext-log4sp/Linux-testFile.log");
    locLinux.line = 123;
    strcopy(locLinux.funcname, sizeof(SourceLoc::funcname), "Function1");

    SourceLoc locWin;
    strcopy(locWin.filename, sizeof(SourceLoc::filename), "C:\\user\\sm-ext-log4sp\\Win-testFile.log");
    locWin.line = 456;
    strcopy(locWin.funcname, sizeof(SourceLoc::funcname), "Function2");

    char expectedLinux[2048];
    FormatEx(expectedLinux, sizeof(expectedLinux), P_PREFIX ... "\\[Linux-testFile.log:123\\] test message (1|2|3)");

    char expectedWin[2048];
    FormatEx(expectedWin, sizeof(expectedWin), P_PREFIX ... "\\[Win-testFile.log:456\\] test message (1|2|3)");

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.LogLoc(locLinux.filename, locLinux.line, locLinux.funcname, LogLevel_Info, "test message 1");
    AssertStrMatch("Linux LogLoc line match", sink.DrainLastLine(), expectedLinux);

    logger.LogLocEx(locLinux.filename, locLinux.line, locLinux.funcname, LogLevel_Info, "test message %d", 2);
    AssertStrMatch("Linux LogLocEx line match", sink.DrainLastLine(), expectedLinux);

    logger.LogLocAmxTpl(locLinux.filename, locLinux.line, locLinux.funcname, LogLevel_Info, "test message %d", 3);
    AssertStrMatch("Linux LogLocAmxTpl line match", sink.DrainLastLine(), expectedLinux);


    logger.LogLoc(locWin.filename, locWin.line, locWin.funcname, LogLevel_Info, "test message 1");
    AssertStrMatch("Win LogLoc line match", sink.DrainLastLine(), expectedWin);

    logger.LogLocEx(locWin.filename, locWin.line, locWin.funcname, LogLevel_Info, "test message %d", 2);
    AssertStrMatch("Win LogLocEx line match", sink.DrainLastLine(), expectedWin);

    logger.LogLocAmxTpl(locWin.filename, locWin.line, locWin.funcname, LogLevel_Info, "test message %d", 3);
    AssertStrMatch("Win LogLocAmxTpl line match", sink.DrainLastLine(), expectedWin);

    logger.Close();
    sink.Close();
}


void TestLogStackTrace()
{
    SetTestContext("Test Logger LogStackTrace");

    char path[PLATFORM_MAX_PATH];
    BuildTestPath(path, sizeof(path), "logger-log/log-stack-trace.log");

    TestSink sink = new TestSink();
    Logger logger = BasicFileSink.CreateLogger(LOGGER_NAME, path);
    logger.AddSink(sink);

    logger.LogStackTrace(LogLevel_Info, "test message 1");
    logger.LogStackTraceEx(LogLevel_Info, "test message %d", 2);
    logger.LogStackTraceAmxTpl(LogLevel_Info, "test message %d", 3);
    delete logger;

    AssertStrMatch("LogStackTraceAmxTpl line 6 match", sink.DrainLastLine(), P_PREFIX ... "  \\[2\\] Line [0-9]+, .*test-logger-log.sp::Command_Test");
    AssertStrMatch("LogStackTraceAmxTpl line 5 match", sink.DrainLastLine(), P_PREFIX ... "  \\[1\\] Line [0-9]+, .*test-logger-log.sp::TestLogStackTrac");
    AssertStrMatch("LogStackTraceAmxTpl line 4 match", sink.DrainLastLine(), P_PREFIX ... "  \\[0\\] Logger.LogStackTraceAmxTpl");
    AssertStrMatch("LogStackTraceAmxTpl line 3 match", sink.DrainLastLine(), P_PREFIX ... "Call stack trace:");
    AssertStrMatch("LogStackTraceAmxTpl line 2 match", sink.DrainLastLine(), P_PREFIX ... "Called from: .*test-logger-log.smx");
    AssertStrMatch("LogStackTraceAmxTpl line 1 match", sink.DrainLastLine(), P_PREFIX ... "Stack trace requested: test message 3");

    AssertStrMatch("LogStackTraceEx line 6 match", sink.DrainLastLine(), P_PREFIX ... "  \\[2\\] Line [0-9]+, .*test-logger-log.sp::Command_Test");
    AssertStrMatch("LogStackTraceEx line 5 match", sink.DrainLastLine(), P_PREFIX ... "  \\[1\\] Line [0-9]+, .*test-logger-log.sp::TestLogStackTrace");
    AssertStrMatch("LogStackTraceEx line 4 match", sink.DrainLastLine(), P_PREFIX ... "  \\[0\\] Logger.LogStackTraceE");
    AssertStrMatch("LogStackTraceEx line 3 match", sink.DrainLastLine(), P_PREFIX ... "Call stack trace:");
    AssertStrMatch("LogStackTraceEx line 2 match", sink.DrainLastLine(), P_PREFIX ... "Called from: .*test-logger-log.smx");
    AssertStrMatch("LogStackTraceEx line 1 match", sink.DrainLastLine(), P_PREFIX ... "Stack trace requested: test message 2");

    AssertStrMatch("LogStackTrace line 6 match", sink.DrainLastLine(), P_PREFIX ... "  \\[2\\] Line [0-9]+, .*test-logger-log.sp::Command_Test");
    AssertStrMatch("LogStackTrace line 5 match", sink.DrainLastLine(), P_PREFIX ... "  \\[1\\] Line [0-9]+, .*test-logger-log.sp::TestLogStackTrace");
    AssertStrMatch("LogStackTrace line 4 match", sink.DrainLastLine(), P_PREFIX ... "  \\[0\\] Logger.LogStackTrace");
    AssertStrMatch("LogStackTrace line 3 match", sink.DrainLastLine(), P_PREFIX ... "Call stack trace:");
    AssertStrMatch("LogStackTrace line 2 match", sink.DrainLastLine(), P_PREFIX ... "Called from: .*test-logger-log.smx");
    AssertStrMatch("LogStackTrace line 1 match", sink.DrainLastLine(), P_PREFIX ... "Stack trace requested: test message 1");
    sink.Close();
}

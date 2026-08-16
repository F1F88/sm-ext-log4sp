#pragma semicolon 1
#pragma newdecls required

/**
 * Fix: "Not enough space on the heap"
 * 似乎是由于 TestSink.DrainLastLineFast 的调用堆栈过深
 */
#pragma dynamic 131072

#include <sourcemod>
#include <log4sp>

#include "../test_sink"
#include "../test_utils"


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
    AssertStrMatch("Log line match", sink.DrainLastLineFast(), P_PREFIX ... "test message 1");

    logger.LogEx(LogLevel_Info, "test message %d", 2);
    AssertStrMatch("LogEx line match", sink.DrainLastLineFast(), P_PREFIX ... "test message 2");

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
    AssertStrMatch("LogSrc line match", sink.DrainLastLineFast(), P_PREFIX ... "\\[test-logger-log.sp:[0-9]+\\] test message 1");

    logger.LogSrcEx(LogLevel_Info, "test message %d", 2);
    AssertStrMatch("LogSrcEx line match", sink.DrainLastLineFast(), P_PREFIX ... "\\[test-logger-log.sp:[0-9]+\\] test message 2");

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

    logger.LogLoc(locLinux, LogLevel_Info, "test message 1");
    AssertStrMatch("Linux LogLoc line match", sink.DrainLastLineFast(), expectedLinux);

    logger.LogLocEx(locLinux, LogLevel_Info, "test message %d", 2);
    AssertStrMatch("Linux LogLocEx line match", sink.DrainLastLineFast(), expectedLinux);


    logger.LogLoc(locWin, LogLevel_Info, "test message 1");
    AssertStrMatch("Win LogLoc line match", sink.DrainLastLineFast(), expectedWin);

    logger.LogLocEx(locWin, LogLevel_Info, "test message %d", 2);
    AssertStrMatch("Win LogLocEx line match", sink.DrainLastLineFast(), expectedWin);

    logger.Close();
    sink.Close();
}


void TestLogStackTrace()
{
    SetTestContext("Test Logger LogStackTrace");

    char path[PLATFORM_MAX_PATH];
    BuildTestPath(path, sizeof(path), "logger-log/log-stack-trace.log");

    TestSink testSink = new TestSink();
    BasicFileSink basicFileSink = new BasicFileSink(path);
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(testSink);
    logger.AddSink(basicFileSink);

    logger.LogStackTrace(LogLevel_Info, "test message 1");
    logger.LogStackTraceEx(LogLevel_Info, "test message %d", 2);
    delete logger;
    delete basicFileSink;

    AssertStrMatch("LogStackTraceEx line 6 match", testSink.DrainLastLineFast(), P_PREFIX ... "  \\[2\\] Line [0-9]+, .*test-logger-log.sp::Command_Test");
    AssertStrMatch("LogStackTraceEx line 5 match", testSink.DrainLastLineFast(), P_PREFIX ... "  \\[1\\] Line [0-9]+, .*test-logger-log.sp::TestLogStackTrace");
    AssertStrMatch("LogStackTraceEx line 4 match", testSink.DrainLastLineFast(), P_PREFIX ... "  \\[0\\] Logger.LogStackTraceEx");
    AssertStrMatch("LogStackTraceEx line 3 match", testSink.DrainLastLineFast(), P_PREFIX ... "Call stack trace:");
    AssertStrMatch("LogStackTraceEx line 2 match", testSink.DrainLastLineFast(), P_PREFIX ... "Called from: test-logger-log.smx");
    AssertStrMatch("LogStackTraceEx line 1 match", testSink.DrainLastLineFast(), P_PREFIX ... "Stack trace requested: test message 2");

    AssertStrMatch("LogStackTrace line 6 match", testSink.DrainLastLineFast(), P_PREFIX ... "  \\[2\\] Line [0-9]+, .*test-logger-log.sp::Command_Test");
    AssertStrMatch("LogStackTrace line 5 match", testSink.DrainLastLineFast(), P_PREFIX ... "  \\[1\\] Line [0-9]+, .*test-logger-log.sp::TestLogStackTrace");
    AssertStrMatch("LogStackTrace line 4 match", testSink.DrainLastLineFast(), P_PREFIX ... "  \\[0\\] Logger.LogStackTrace");
    AssertStrMatch("LogStackTrace line 3 match", testSink.DrainLastLineFast(), P_PREFIX ... "Call stack trace:");
    AssertStrMatch("LogStackTrace line 2 match", testSink.DrainLastLineFast(), P_PREFIX ... "Called from: test-logger-log.smx");
    AssertStrMatch("LogStackTrace line 1 match", testSink.DrainLastLineFast(), P_PREFIX ... "Stack trace requested: test message 1");
    delete testSink;
}

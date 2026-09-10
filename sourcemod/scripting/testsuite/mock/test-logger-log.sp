#pragma semicolon 1
#pragma newdecls required
#pragma dynamic 32768           // Fix: "Not enough space on the heap"

#if !defined DEBUG
    #define  DEBUG
#endif

#if !defined _DEBUG
    #define  _DEBUG
#endif

#if defined NDEBUG
    #undef  NDEBUG
#endif

#include <regex>
#include <sourcemod>
#include <log4sp>

#include "../assert"
#include "../test_sink"
#include "../test_utils"


native void TestLoggerLogStackTrace(Logger logger);
native void TestLoggerLogStackTrace1(Logger logger);
native void TestLoggerLogStackTrace2(Logger logger);
native void TestLoggerLogStackTrace3(Logger logger);


public APLRes AskPluginLoad2(Handle myself, bool late, char[] error, int err_max)
{
    CreateNative("TestLoggerLogStackTrace",  Native_TestLoggerLogStackTrace);
    CreateNative("TestLoggerLogStackTrace1", Native_TestLoggerLogStackTrace1);
    CreateNative("TestLoggerLogStackTrace2", Native_TestLoggerLogStackTrace2);
    CreateNative("TestLoggerLogStackTrace3", Native_TestLoggerLogStackTrace3);
    return APLRes_Success;
}

public void OnPluginStart()
{
    Test();
    RegServerCmd("sm_log4sp_test_logger_log", Command_Test);
}

Action Command_Test(int args)
{
    Test();
    return Plugin_Handled;
}


void Test()
{
    PrintToServer("----------- Started testing Logger-Log -----------");

    TestLog();

    TestLogSrc();

    TestLogLoc();

    TestLogStackTrace();

    PrintToServer("-------------- Test Logger-Log ended -------------");
}

void TestLog()
{
    SetTestContext("Logger.Log");

    char LOGGER_NAME[] = "Test-Log";

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.Log(LogLevel_Info, "test message 1");
    logger.Log(LogLevel_Info, "test message 2");
    logger.Log(LogLevel_Info, "test message 3");
    logger.LogF(LogLevel_Info, "test message %d", 4);
    logger.LogF(LogLevel_Info, "test message %d", 5);
    logger.LogF(LogLevel_Info, "test message %d", 6);

    AssertStrEq("LogF msg", sink.DrainLatest().msg, "test message 6");
    AssertStrEq("LogF name", sink.DrainLatest().name, LOGGER_NAME);
    AssertEq("LogF lvl", sink.DrainLatest().lvl, LogLevel_Info);

    AssertEq("Log line", sink.DrainLatest().loc.line, 0);
    AssertStrEq("Log funcname", sink.DrainLatest().loc.funcname, "");
    AssertStrEq("Log filename", sink.DrainLatest().loc.filename, "");

    SinkCleanupAndDelete(sink);
    LoggerCleanupAndDelete(logger);
}

void TestLogSrc()
{
    SetTestContext("Logger.LogSrc");

    char LOGGER_NAME[] = "Test-LogSrc";

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.LogSrc(LogLevel_Warn, "test message 1");
    logger.LogSrc(LogLevel_Warn, "test message 2");
    logger.LogSrc(LogLevel_Warn, "test message 3");
    logger.LogSrcF(LogLevel_Warn, "test message %d", 4);
    logger.LogSrcF(LogLevel_Warn, "test message %d", 5);
    logger.LogSrcF(LogLevel_Warn, "test message %d", 6);

    AssertStrEq("LogSrcF msg", sink.DrainLatest().msg, "test message 6");
    AssertStrEq("LogSrcF name", sink.DrainLatest().name, LOGGER_NAME);
    AssertEq("LogSrcF lvl", sink.DrainLatest().lvl, LogLevel_Warn);

    AssertEq("LogSrc line", sink.DrainLatest().loc.line, __LINE__ - 9);
    AssertStrEq("LogSrc funcname", sink.DrainLatest().loc.funcname, "TestLogSrc");
    AssertStrEndsWith("LogSrc filename endwith", sink.DrainLatest().loc.filename, "test-logger-log.sp");

    LoggerCleanupAndDelete(logger);
    SinkCleanupAndDelete(sink);
}

void TestLogLoc()
{
    SetTestContext("Logger.LogLoc");

    char LOGGER_NAME[] = "Test-LogSrc";

    const int LINUX_LINE  = 123;
    char[] LINUX_FILENAME = "/home/sm-ext-log4sp/Linux-testFile.log";
    char[] LINUX_FUNCNAME = "Function1";

    const int WIN_LINE  = 456;
    char[] WIN_FILENAME = "C:\\user\\sm-ext-log4sp\\Win-testFile.log";
    char[] WIN_FUNCNAME = "Function2";

    SourceLoc linuxLoc;
    linuxLoc.line = LINUX_LINE;
    strcopy(linuxLoc.filename, sizeof(SourceLoc::filename), LINUX_FILENAME);
    strcopy(linuxLoc.funcname, sizeof(SourceLoc::funcname), LINUX_FUNCNAME);

    SourceLoc winLoc;
    winLoc.line = WIN_LINE;
    strcopy(winLoc.filename, sizeof(SourceLoc::filename), WIN_FILENAME);
    strcopy(winLoc.funcname, sizeof(SourceLoc::funcname), WIN_FUNCNAME);

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.LogLoc(linuxLoc, LogLevel_Error, "test message 1");
    logger.LogLoc(linuxLoc, LogLevel_Error, "test message 2");
    logger.LogLoc(linuxLoc, LogLevel_Error, "test message 3");
    logger.LogLocF(linuxLoc, LogLevel_Error, "test message %d", 4);
    logger.LogLocF(linuxLoc, LogLevel_Error, "test message %d", 5);
    logger.LogLocF(linuxLoc, LogLevel_Error, "test message %d", 6);
    logger.LogLoc(winLoc, LogLevel_Warn, "test message 1");
    logger.LogLoc(winLoc, LogLevel_Warn, "test message 2");
    logger.LogLoc(winLoc, LogLevel_Warn, "test message 3");
    logger.LogLocF(winLoc, LogLevel_Error, "test message %d", 4);
    logger.LogLocF(winLoc, LogLevel_Error, "test message %d", 5);
    logger.LogLocF(winLoc, LogLevel_Error, "test message %d", 6);

    AssertStrEq("LogLocF win msg", sink.DrainLatest().msg, "test message 6");
    AssertStrEq("LogLocF win name", sink.DrainLatest().name, LOGGER_NAME);
    AssertEq("LogLocF win lvl", sink.DrainLatest().lvl, LogLevel_Error);

    AssertEq("LogLocF win line", sink.DrainLatest().loc.line, WIN_LINE);
    AssertStrEq("LogLocF win funcname", sink.DrainLatest().loc.funcname, WIN_FUNCNAME);
    AssertStrEq("LogLocF win filename", sink.DrainLatest().loc.filename, WIN_FILENAME);

    AssertStrEq("LogLoc linux msg", sink.DrainLatest().msg, "test message 6");
    AssertStrEq("LogLoc linux name", sink.DrainLatest().name, LOGGER_NAME);
    AssertEq("LogLoc linux lvl", sink.DrainLatest().lvl, LogLevel_Error);

    AssertEq("LogLoc linux line", sink.DrainLatest().loc.line, LINUX_LINE);
    AssertStrEq("LogLoc linux funcname", sink.DrainLatest().loc.funcname, LINUX_FUNCNAME);
    AssertStrEq("LogLoc linux filename", sink.DrainLatest().loc.filename, LINUX_FILENAME);

    SinkCleanupAndDelete(sink);
    LoggerCleanupAndDelete(logger);
}

void TestLogStackTrace()
{
    SetTestContext("Logger.LogStackTrace");

    TestSink sink = new TestSink();
    Logger logger = new Logger();
    logger.AddSink(sink);

    TestLoggerLogStackTrace(logger);

    AssertStrEq(   "LogStackTrace  head  1", sink.DrainOldest().msg, "Stack trace requested: test message 1");
    AssertStrMatch("LogStackTrace  head  2", sink.DrainOldest().msg, "Called from: .*test-logger-log.smx");
    AssertStrEq(   "LogStackTrace  head  3", sink.DrainOldest().msg, "Call stack trace:");
    AssertStrMatch("LogStackTrace  line  0", sink.DrainOldest().msg, "  \\[[0-9]+\\] Logger.LogStackTrace");
    AssertStrMatch("LogStackTrace  line  1", sink.DrainOldest().msg, "  \\[[0-9]+\\] Line [0-9]+, .*test-logger-log.sp::__Func1");
    AssertStrMatch("LogStackTrace  line  2", sink.DrainOldest().msg, "  \\[[0-9]+\\] Line [0-9]+, .*test-logger-log.sp::__Func2");
    AssertStrMatch("LogStackTrace  line  3", sink.DrainOldest().msg, "  \\[[0-9]+\\] Line [0-9]+, .*test-logger-log.sp::__Func3");
    AssertStrMatch("LogStackTrace  line  4", sink.DrainOldest().msg, "  \\[[0-9]+\\] Line [0-9]+, .*test-logger-log.sp::Native_TestLoggerLogStackTrace1");
    AssertStrMatch("LogStackTrace  line  6", sink.DrainOldest().msg, "  \\[[0-9]+\\] TestLoggerLogStackTrace1");
    AssertStrMatch("LogStackTrace  line  7", sink.DrainOldest().msg, "  \\[[0-9]+\\] Line [0-9]+, .*test-logger-log.sp::Native_TestLoggerLogStackTrace2");
    AssertStrMatch("LogStackTrace  line  9", sink.DrainOldest().msg, "  \\[[0-9]+\\] TestLoggerLogStackTrace2");
    AssertStrMatch("LogStackTrace  line 10", sink.DrainOldest().msg, "  \\[[0-9]+\\] Line [0-9]+, .*test-logger-log.sp::Native_TestLoggerLogStackTrace3");
    AssertStrMatch("LogStackTrace  line 12", sink.DrainOldest().msg, "  \\[[0-9]+\\] TestLoggerLogStackTrace3");
    AssertStrMatch("LogStackTrace  line 13", sink.DrainOldest().msg, "  \\[[0-9]+\\] Line [0-9]+, .*test-logger-log.sp::Native_TestLoggerLogStackTrace");
    AssertStrMatch("LogStackTrace  line 15", sink.DrainOldest().msg, "  \\[[0-9]+\\] TestLoggerLogStackTrace");
    AssertStrMatch("LogStackTrace  line 16", sink.DrainOldest().msg, "  \\[[0-9]+\\] Line [0-9]+, .*test-logger-log.sp::TestLogStackTrace");
    AssertStrMatch("LogStackTrace  line 17", sink.DrainOldest().msg, "  \\[[0-9]+\\] Line [0-9]+, .*test-logger-log.sp::Test");
    AssertStrMatch("LogStackTraceF line 18", sink.DrainOldest().msg, "  \\[[0-9]+\\] Line [0-9]+, .*test-logger-log.sp::[OnPluginStart|Command_Test]");

    // autoreload.smx 可能进入堆栈
    char msg[sizeof(LogEvent::msg)];
    msg = sink.DrainOldest().msg;
    while (strncmp(msg, "Stack trace requested: test message 2", 37))
    {
        msg = sink.DrainOldest().msg;
    }

    AssertStrEq(   "LogStackTraceF head  1",                    msg, "Stack trace requested: test message 2");
    AssertStrMatch("LogStackTraceF head  2", sink.DrainOldest().msg, "Called from: .*test-logger-log.smx");
    AssertStrEq(   "LogStackTraceF head  3", sink.DrainOldest().msg, "Call stack trace:");
    AssertStrMatch("LogStackTraceF line  0", sink.DrainOldest().msg, "  \\[[0-9]+\\] Logger.LogStackTraceF");
    AssertStrMatch("LogStackTraceF line  1", sink.DrainOldest().msg, "  \\[[0-9]+\\] Line [0-9]+, .*test-logger-log.sp::__Func1");
    AssertStrMatch("LogStackTraceF line  2", sink.DrainOldest().msg, "  \\[[0-9]+\\] Line [0-9]+, .*test-logger-log.sp::__Func2");
    AssertStrMatch("LogStackTraceF line  3", sink.DrainOldest().msg, "  \\[[0-9]+\\] Line [0-9]+, .*test-logger-log.sp::__Func3");
    AssertStrMatch("LogStackTraceF line  4", sink.DrainOldest().msg, "  \\[[0-9]+\\] Line [0-9]+, .*test-logger-log.sp::Native_TestLoggerLogStackTrace1");
    AssertStrMatch("LogStackTraceF line  6", sink.DrainOldest().msg, "  \\[[0-9]+\\] TestLoggerLogStackTrace1");
    AssertStrMatch("LogStackTraceF line  7", sink.DrainOldest().msg, "  \\[[0-9]+\\] Line [0-9]+, .*test-logger-log.sp::Native_TestLoggerLogStackTrace2");
    AssertStrMatch("LogStackTraceF line  9", sink.DrainOldest().msg, "  \\[[0-9]+\\] TestLoggerLogStackTrace2");
    AssertStrMatch("LogStackTraceF line 10", sink.DrainOldest().msg, "  \\[[0-9]+\\] Line [0-9]+, .*test-logger-log.sp::Native_TestLoggerLogStackTrace3");
    AssertStrMatch("LogStackTraceF line 12", sink.DrainOldest().msg, "  \\[[0-9]+\\] TestLoggerLogStackTrace3");
    AssertStrMatch("LogStackTraceF line 13", sink.DrainOldest().msg, "  \\[[0-9]+\\] Line [0-9]+, .*test-logger-log.sp::Native_TestLoggerLogStackTrace");
    AssertStrMatch("LogStackTraceF line 15", sink.DrainOldest().msg, "  \\[[0-9]+\\] TestLoggerLogStackTrace");
    AssertStrMatch("LogStackTraceF line 16", sink.DrainOldest().msg, "  \\[[0-9]+\\] Line [0-9]+, .*test-logger-log.sp::TestLogStackTrace");
    AssertStrMatch("LogStackTraceF line 17", sink.DrainOldest().msg, "  \\[[0-9]+\\] Line [0-9]+, .*test-logger-log.sp::Test");
    AssertStrMatch("LogStackTraceF line 18", sink.DrainOldest().msg, "  \\[[0-9]+\\] Line [0-9]+, .*test-logger-log.sp::[OnPluginStart|Command_Test]");

    LoggerCleanupAndDelete(logger);
    SinkCleanupAndDelete(sink);
}

public any Native_TestLoggerLogStackTrace(Handle plugin, int numParams)
{
    Logger logger = GetNativeCell(1);
    TestLoggerLogStackTrace3(logger);
    return 0;
}

public any Native_TestLoggerLogStackTrace3(Handle plugin, int numParams)
{
    Logger logger = GetNativeCell(1);
    TestLoggerLogStackTrace2(logger);
    return 0;
}

public any Native_TestLoggerLogStackTrace2(Handle plugin, int numParams)
{
    Logger logger = GetNativeCell(1);
    TestLoggerLogStackTrace1(logger);
    return 0;
}

public any Native_TestLoggerLogStackTrace1(Handle plugin, int numParams)
{
    Logger logger = GetNativeCell(1);
    __Func3(logger);
    return 0;
}

static void __Func3(Logger logger)
{
    __Func2(logger);
}

static void __Func2(Logger logger)
{
    __Func1(logger);
}

static void __Func1(Logger logger)
{
    logger.LogStackTrace(LogLevel_Info, "test message 1");
    logger.LogStackTraceF(LogLevel_Info, "test message %d", 2);
}

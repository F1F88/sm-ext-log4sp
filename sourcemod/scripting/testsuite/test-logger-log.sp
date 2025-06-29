#pragma semicolon 1
#pragma newdecls required

/**
 * Fix: "Not enough space on the heap"
 * 似乎是由于 TestSink.DrainLastLineFast 的调用堆栈过深
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

    TestLogThrowError();

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

    logger.LogAmxTpl(LogLevel_Info, "test message %d", 3);
    AssertStrMatch("LogAmxTpl line match", sink.DrainLastLineFast(), P_PREFIX ... "test message 3");

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

    logger.LogSrcAmxTpl(LogLevel_Info, "test message %d", 3);
    AssertStrMatch("LogSrcAmxTpl line match", sink.DrainLastLineFast(), P_PREFIX ... "\\[test-logger-log.sp:[0-9]+\\] test message 3");

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
    AssertStrMatch("Linux LogLoc line match", sink.DrainLastLineFast(), expectedLinux);

    logger.LogLocEx(locLinux.filename, locLinux.line, locLinux.funcname, LogLevel_Info, "test message %d", 2);
    AssertStrMatch("Linux LogLocEx line match", sink.DrainLastLineFast(), expectedLinux);

    logger.LogLocAmxTpl(locLinux.filename, locLinux.line, locLinux.funcname, LogLevel_Info, "test message %d", 3);
    AssertStrMatch("Linux LogLocAmxTpl line match", sink.DrainLastLineFast(), expectedLinux);


    logger.LogLoc(locWin.filename, locWin.line, locWin.funcname, LogLevel_Info, "test message 1");
    AssertStrMatch("Win LogLoc line match", sink.DrainLastLineFast(), expectedWin);

    logger.LogLocEx(locWin.filename, locWin.line, locWin.funcname, LogLevel_Info, "test message %d", 2);
    AssertStrMatch("Win LogLocEx line match", sink.DrainLastLineFast(), expectedWin);

    logger.LogLocAmxTpl(locWin.filename, locWin.line, locWin.funcname, LogLevel_Info, "test message %d", 3);
    AssertStrMatch("Win LogLocAmxTpl line match", sink.DrainLastLineFast(), expectedWin);

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

    AssertStrMatch("LogStackTraceAmxTpl line 6 match", sink.DrainLastLineFast(), P_PREFIX ... "  \\[2\\] Line [0-9]+, .*test-logger-log.sp::Command_Test");
    AssertStrMatch("LogStackTraceAmxTpl line 5 match", sink.DrainLastLineFast(), P_PREFIX ... "  \\[1\\] Line [0-9]+, .*test-logger-log.sp::TestLogStackTrac");
    AssertStrMatch("LogStackTraceAmxTpl line 4 match", sink.DrainLastLineFast(), P_PREFIX ... "  \\[0\\] Logger.LogStackTraceAmxTpl");
    AssertStrMatch("LogStackTraceAmxTpl line 3 match", sink.DrainLastLineFast(), P_PREFIX ... "Call stack trace:");
    AssertStrMatch("LogStackTraceAmxTpl line 2 match", sink.DrainLastLineFast(), P_PREFIX ... "Called from: test-logger-log.smx");
    AssertStrMatch("LogStackTraceAmxTpl line 1 match", sink.DrainLastLineFast(), P_PREFIX ... "Stack trace requested: test message 3");

    AssertStrMatch("LogStackTraceEx line 6 match", sink.DrainLastLineFast(), P_PREFIX ... "  \\[2\\] Line [0-9]+, .*test-logger-log.sp::Command_Test");
    AssertStrMatch("LogStackTraceEx line 5 match", sink.DrainLastLineFast(), P_PREFIX ... "  \\[1\\] Line [0-9]+, .*test-logger-log.sp::TestLogStackTrace");
    AssertStrMatch("LogStackTraceEx line 4 match", sink.DrainLastLineFast(), P_PREFIX ... "  \\[0\\] Logger.LogStackTraceE");
    AssertStrMatch("LogStackTraceEx line 3 match", sink.DrainLastLineFast(), P_PREFIX ... "Call stack trace:");
    AssertStrMatch("LogStackTraceEx line 2 match", sink.DrainLastLineFast(), P_PREFIX ... "Called from: test-logger-log.smx");
    AssertStrMatch("LogStackTraceEx line 1 match", sink.DrainLastLineFast(), P_PREFIX ... "Stack trace requested: test message 2");

    AssertStrMatch("LogStackTrace line 6 match", sink.DrainLastLineFast(), P_PREFIX ... "  \\[2\\] Line [0-9]+, .*test-logger-log.sp::Command_Test");
    AssertStrMatch("LogStackTrace line 5 match", sink.DrainLastLineFast(), P_PREFIX ... "  \\[1\\] Line [0-9]+, .*test-logger-log.sp::TestLogStackTrace");
    AssertStrMatch("LogStackTrace line 4 match", sink.DrainLastLineFast(), P_PREFIX ... "  \\[0\\] Logger.LogStackTrace");
    AssertStrMatch("LogStackTrace line 3 match", sink.DrainLastLineFast(), P_PREFIX ... "Call stack trace:");
    AssertStrMatch("LogStackTrace line 2 match", sink.DrainLastLineFast(), P_PREFIX ... "Called from: test-logger-log.smx");
    AssertStrMatch("LogStackTrace line 1 match", sink.DrainLastLineFast(), P_PREFIX ... "Stack trace requested: test message 1");
    delete sink;
}


void TestLogThrowError()
{
    SetTestContext("Test Logger ThrowError");

    char path[PLATFORM_MAX_PATH];
    BuildTestPath(path, sizeof(path), "logger-log/throw-error.log");

    TestSink sink = new TestSink();
    Logger logger = BasicFileSink.CreateLogger(LOGGER_NAME, path);
    logger.AddSink(sink);
    logger.FlushOn(LogLevel_Info);

    RequestFrame(Frame_MarkStart);
    RequestFrame(Frame_ThorwErrorDebug, logger);
    RequestFrame(Frame_ThorwErrorExDebug, logger);
    RequestFrame(Frame_ThorwErrorAmxTplDebug, logger);

    RequestFrame(Frame_ThorwErrorInfo, logger);
    RequestFrame(Frame_ThorwErrorExInfo, logger);
    RequestFrame(Frame_ThorwErrorAmxTplInfo, logger);
    RequestFrame(Frame_CloseLogger, logger);
    RequestFrame(Frame_MarkEnd);

    RequestFrame(Frame_AssertThrowErrorSinkMsgs, sink);
    RequestFrame(Frame_AssertThrowErrorLogFile);
    RequestFrame(Frame_AssertThrowErrorSMFile); // And DeleteSMErrorFile
    RequestFrame(Frame_CloseSink, sink);
}

static void Frame_MarkStart()
{
    MarkErrorTestStart("Test Log ThrowError");
}

static void Frame_MarkEnd()
{
    MarkErrorTestEnd("Test Log ThrowError");
}

void Frame_ThorwErrorDebug(Logger logger)
{
    logger.ThrowError(LogLevel_Debug, "test message 1");
}

void Frame_ThorwErrorExDebug(Logger logger)
{
    logger.ThrowErrorEx(LogLevel_Debug, "test message %d", 2);
}

void Frame_ThorwErrorAmxTplDebug(Logger logger)
{
    logger.ThrowErrorAmxTpl(LogLevel_Debug, "test message %d", 3);
}

void Frame_ThorwErrorInfo(Logger logger)
{
    logger.ThrowError(LogLevel_Info, "test message 1");
}

void Frame_ThorwErrorExInfo(Logger logger)
{
    logger.ThrowErrorEx(LogLevel_Info, "test message %d", 2);
}

void Frame_ThorwErrorAmxTplInfo(Logger logger)
{
    logger.ThrowErrorAmxTpl(LogLevel_Info, "test message %d", 3);
}

void Frame_CloseLogger(Logger logger)
{
    logger.Close();
}

void Frame_CloseSink(Sink sink)
{
    sink.Close();
}

static void Frame_AssertThrowErrorSinkMsgs(TestSink sink)
{
    AssertEq("ThrowError msgs", sink.GetLogCount(), 3 * 5);

    ArrayList messages = sink.DrainMsgsFast();
    sLogMessage logMsg;
    for (int i = 0; i < 3; ++i)
    {
        messages.GetArray(i * 5, logMsg);
        AssertStrMatch("ThrowError sink msg 1 match", logMsg.msg, "Exception reported: test message (1|2|3)");

        messages.GetArray(i * 5 + 1, logMsg);
        AssertStrEq("ThrowError sink msg 2", logMsg.msg, "Blaming: test-logger-log.smx");

        messages.GetArray(i * 5 + 2, logMsg);
        AssertStrEq("ThrowError sink msg 3", logMsg.msg, "Call stack trace:");

        messages.GetArray(i * 5 + 3, logMsg);
        AssertStrMatch("ThrowError sink msg 4 match", logMsg.msg, "  \\[0\\] Logger.ThrowError(Ex|AmxTpl)*");

        messages.GetArray(i * 5 + 4, logMsg);
        AssertStrMatch("ThrowError sink msg 5 match", logMsg.msg, "  \\[1\\] Line [0-9]+, .*test-logger-log.sp::Frame_ThorwError(Ex|AmxTpl)*(Debug|Info)");
    }
    delete messages;
}

static void Frame_AssertThrowErrorLogFile()
{
#define THROW_ERROR_LOG_FILE_EXP   "(?:" ... P_PREFIX ... "Exception reported: test message (1|2|3)\
(\n|\r\n)" ... P_PREFIX ... "Blaming: test-logger-log.smx\
(\n|\r\n)" ... P_PREFIX ... "Call stack trace:\
(\n|\r\n)" ... P_PREFIX ... "  \\[0\\] Logger.ThrowError(Ex|AmxTpl)*\
(\n|\r\n)" ... P_PREFIX ... "  \\[1\\] Line [0-9]+, .*test-logger-log.sp::Frame_ThorwError(Ex|AmxTpl)*(Debug|Info)(\n|\r\n)){3}"

    char path[PLATFORM_MAX_PATH];
    BuildTestPath(path, sizeof(path), "logger-log/throw-error.log");
    AssertFileMatch("ThrowError log file match", path, THROW_ERROR_LOG_FILE_EXP);
}

static void Frame_AssertThrowErrorSMFile()
{
#define THROW_ERROR_SM_FILE_EXP   "(?:" ... P_SM_PREFIX ... "Exception reported: test message (1|2|3)\
(\n|\r\n)" ... P_SM_PREFIX ... "Blaming: test-logger-log.smx\
(\n|\r\n)" ... P_SM_PREFIX ... "Call stack trace:\
(\n|\r\n)" ... P_SM_PREFIX ... "  \\[0\\] Logger.ThrowError(Ex|AmxTpl)*\
(\n|\r\n)" ... P_SM_PREFIX ... "  \\[1\\] Line [0-9]+, .*test-logger-log.sp::Frame_ThorwError(Ex|AmxTpl)*(Debug|Info)(\n|\r\n)){6}"

    AssertFileMatch("ThrowError sm file match", GetErrorFilename(), THROW_ERROR_SM_FILE_EXP);

    // 若检验通过，则删除本次测试生成的日志信息以保持 SM 错误日志的简洁
    // DeleteFile(GetErrorFilename());
}

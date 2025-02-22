#include <log4sp>

#include "test_sink"

#pragma semicolon 1
#pragma newdecls required

#define LOGGER_NAME             "test-log"


public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_log", Command_Test);
}

Action Command_Test(int args)
{
    PrintToServer("---- START TEST LOG ----");

    SetTestContext("Test Logger Log");

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
    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.Log(LogLevel_Info, "test message 1");
    AssertStrMatch("Log", sink.GetLastLogLine(), "\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] test message 1\\s");

    logger.LogEx(LogLevel_Info, "test message %d", 2);
    AssertStrMatch("LogEx", sink.GetLastLogLine(), "\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] test message 2\\s");

    logger.LogAmxTpl(LogLevel_Info, "test message %d", 3);
    AssertStrMatch("LogAmxTpl", sink.GetLastLogLine(), "\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] test message 3\\s");

    logger.Close();
    sink.Close();
}


void TestLogSrc()
{
    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.LogSrc(LogLevel_Info, "test message 1");
    AssertStrMatch("LogSrc", sink.GetLastLogLine(), "\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] \\[test-logger-log.sp:[0-9]+\\] test message 1\\s");

    logger.LogSrcEx(LogLevel_Info, "test message %d", 2);
    AssertStrMatch("LogSrcEx", sink.GetLastLogLine(), "\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] \\[test-logger-log.sp:[0-9]+\\] test message 2\\s");

    logger.LogSrcAmxTpl(LogLevel_Info, "test message %d", 3);
    AssertStrMatch("LogSrcAmxTpl", sink.GetLastLogLine(), "\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] \\[test-logger-log.sp:[0-9]+\\] test message 3\\s");

    logger.Close();
    sink.Close();
}


void TestLogLoc()
{
#define TEST_LOG_LOC_FILE1  "/home/sm-ext-log4sp/Linux-testFile.log"
#define TEST_LOG_LOC_LINE1  123
#define TEST_LOG_LOC_FUNC1  "Function1"
#define TEST_LOG_LOC_EXP1   "\\[Linux-testFile\\.log:123\\] "

#define TEST_LOG_LOC_FILE2  "C:\\user\\sm-ext-log4sp\\Win-testFile.log"
#define TEST_LOG_LOC_LINE2  456
#define TEST_LOG_LOC_FUNC2  "Function2"
#define TEST_LOG_LOC_EXP2   "\\[Win-testFile\\.log:456\\] "

    TestSink sink = new TestSink();
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.LogLoc(TEST_LOG_LOC_FILE1, TEST_LOG_LOC_LINE1, TEST_LOG_LOC_FUNC1, LogLevel_Info, "test message 1");
    AssertStrMatch("LogLoc", sink.GetLastLogLine(), "\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] " ... TEST_LOG_LOC_EXP1 ... "test message 1\\s");

    logger.LogLocEx(TEST_LOG_LOC_FILE1, TEST_LOG_LOC_LINE1, TEST_LOG_LOC_FUNC1, LogLevel_Info, "test message %d", 2);
    AssertStrMatch("LogLocEx", sink.GetLastLogLine(), "\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] " ... TEST_LOG_LOC_EXP1 ... "test message 2\\s");

    logger.LogLocAmxTpl(TEST_LOG_LOC_FILE1, TEST_LOG_LOC_LINE1, TEST_LOG_LOC_FUNC1, LogLevel_Info, "test message %d", 3);
    AssertStrMatch("LogLocAmxTpl", sink.GetLastLogLine(), "\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] " ... TEST_LOG_LOC_EXP1 ... "test message 3\\s");


    logger.LogLoc(TEST_LOG_LOC_FILE2, TEST_LOG_LOC_LINE2, TEST_LOG_LOC_FUNC2, LogLevel_Info, "test message 1");
    AssertStrMatch("LogLoc", sink.GetLastLogLine(), "\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] " ... TEST_LOG_LOC_EXP2 ... "test message 1\\s");

    logger.LogLocEx(TEST_LOG_LOC_FILE2, TEST_LOG_LOC_LINE2, TEST_LOG_LOC_FUNC2, LogLevel_Info, "test message %d", 2);
    AssertStrMatch("LogLocEx", sink.GetLastLogLine(), "\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] " ... TEST_LOG_LOC_EXP2 ... "test message 2\\s");

    logger.LogLocAmxTpl(TEST_LOG_LOC_FILE2, TEST_LOG_LOC_LINE2, TEST_LOG_LOC_FUNC2, LogLevel_Info, "test message %d", 3);
    AssertStrMatch("LogLocAmxTpl", sink.GetLastLogLine(), "\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] " ... TEST_LOG_LOC_EXP2 ... "test message 3\\s");

    logger.Close();
    sink.Close();
}


void TestLogStackTrace()
{
#define TEST_LOG_STACK_TRACE_EXP "\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] Stack trace requested: test message 1\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] Called from: test-logger-log\\.smx\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] Call stack trace:\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\]   \\[0\\] Logger\\.LogStackTrace\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\]   \\[1\\] Line [0-9]+, .*test-logger-log\\.sp::TestLogStackTrace\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\]   \\[2\\] Line [0-9]+, .*test-logger-log\\.sp::Command_Test\\s"

#define TEST_LOG_STACK_TRACE_EX_EXP "\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] Stack trace requested: test message 2\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] Called from: test-logger-log\\.smx\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] Call stack trace:\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\]   \\[0\\] Logger\\.LogStackTraceEx\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\]   \\[1\\] Line [0-9]+, .*test-logger-log\\.sp::TestLogStackTrace\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\]   \\[2\\] Line [0-9]+, .*test-logger-log\\.sp::Command_Test\\s"

#define TEST_LOG_STACK_TRACE_AMXTPL_EXP "\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] Stack trace requested: test message 3\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] Called from: test-logger-log\\.smx\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] Call stack trace:\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\]   \\[0\\] Logger\\.LogStackTraceAmxTpl\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\]   \\[1\\] Line [0-9]+, .*test-logger-log\\.sp::TestLogStackTrace\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\]   \\[2\\] Line [0-9]+, .*test-logger-log\\.sp::Command_Test\\s"

    char path[PLATFORM_MAX_PATH];
    path = PrepareTestPath("logger-log/log-stack-trace.log");

    Logger logger = BasicFileSink.CreateLogger(LOGGER_NAME, path);

    logger.LogStackTrace(LogLevel_Info, "test message 1");
    logger.LogStackTraceEx(LogLevel_Info, "test message %d", 2);
    logger.LogStackTraceAmxTpl(LogLevel_Info, "test message %d", 3);

    logger.Close();

    AssertFileMatch("LogStackTrace", path, TEST_LOG_STACK_TRACE_EXP);
    AssertFileMatch("LogStackTraceEx", path, TEST_LOG_STACK_TRACE_EX_EXP);
    AssertFileMatch("LogStackTraceAmxTpl", path, TEST_LOG_STACK_TRACE_AMXTPL_EXP);
}


void TestLogThrowError()
{
    LogError("======= Test Log Thorw Error =======");

    // SM 错误日志文件路径
    char errFilePath[PLATFORM_MAX_PATH];
    FormatTime(errFilePath, sizeof(errFilePath), "addons/sourcemod/logs/errors_%Y%m%d.log");

    // 读取当前位置
    File preErrFile = OpenFile(errFilePath, "r");
    preErrFile.Seek(0, SEEK_END);
    int preErrFilePosition = preErrFile.Position;
    delete preErrFile;

    char path[PLATFORM_MAX_PATH];
    path = PrepareTestPath("logger-log/throw-error.log");

    Logger logger = BasicFileSink.CreateLogger(LOGGER_NAME, path);

    RequestFrame(Frame_ThorwErrorDebug, logger);
    RequestFrame(Frame_ThorwErrorExDebug, logger);
    RequestFrame(Frame_ThorwErrorAmxTplDebug, logger);

    RequestFrame(Frame_ThorwErrorInfo, logger);
    RequestFrame(Frame_ThorwErrorExInfo, logger);
    RequestFrame(Frame_ThorwErrorAmxTplInfo, logger);

    RequestFrame(Frame_CloseLogger, logger);

    RequestFrame(Frame_AssertThorwErrorFile, preErrFilePosition);
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

void Frame_AssertThorwErrorFile(int preErrFilePosition)
{
#define TEST_LOG_THROW_ERROR_DEBUG_SM_EXP "\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\] Exception reported: test message 1\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\] Blaming: test-logger-log\\.smx\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\] Call stack trace:\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\]   \\[0\\] Logger\\.ThrowError\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\]   \\[1\\] Line [0-9]+, .*test-logger-log\\.sp::Frame_ThorwErrorDebug\\s"

#define TEST_LOG_THROW_ERROR_EX_DEBUG_SM_EXP "\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\] Exception reported: test message 2\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\] Blaming: test-logger-log\\.smx\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\] Call stack trace:\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\]   \\[0\\] Logger\\.ThrowErrorEx\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\]   \\[1\\] Line [0-9]+, .*test-logger-log\\.sp::Frame_ThorwErrorExDebug\\s"

#define TEST_LOG_THROW_ERROR_AMXTPL_DEBUG_SM_EXP "\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\] Exception reported: test message 3\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\] Blaming: test-logger-log\\.smx\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\] Call stack trace:\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\]   \\[0\\] Logger\\.ThrowErrorAmxTpl\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\]   \\[1\\] Line [0-9]+, .*test-logger-log\\.sp::Frame_ThorwErrorAmxTplDebug\\s"

#define TEST_LOG_THROW_ERROR_INFO_SM_EXP "\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\] Exception reported: test message 1\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\] Blaming: test-logger-log\\.smx\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\] Call stack trace:\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\]   \\[0\\] Logger\\.ThrowError\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\]   \\[1\\] Line [0-9]+, .*test-logger-log\\.sp::Frame_ThorwErrorInfo\\s"

#define TEST_LOG_THROW_ERROR_EX_INFO_SM_EXP "\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\] Exception reported: test message 2\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\] Blaming: test-logger-log\\.smx\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\] Call stack trace:\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\]   \\[0\\] Logger\\.ThrowErrorEx\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\]   \\[1\\] Line [0-9]+, .*test-logger-log\\.sp::Frame_ThorwErrorExInfo\\s"

#define TEST_LOG_THROW_ERROR_AMXTPL_INFO_SM_EXP "\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\] Exception reported: test message 3\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\] Blaming: test-logger-log\\.smx\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\] Call stack trace:\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\]   \\[0\\] Logger\\.ThrowErrorAmxTpl\\s\
L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[SM\\]   \\[1\\] Line [0-9]+, .*test-logger-log\\.sp::Frame_ThorwErrorAmxTplInfo\\s"

    // SM 错误日志文件路径
    char errFilePath[PLATFORM_MAX_PATH];
    FormatTime(errFilePath, sizeof(errFilePath), "addons/sourcemod/logs/errors_%Y%m%d.log");

    // 读取新的数据
    char postMsg[TEST_MAX_MSG_LENGTH * 2];
    File postErrFile = OpenFile(errFilePath, "r");
    postErrFile.Seek(preErrFilePosition, SEEK_SET);
    postErrFile.ReadString(postMsg, sizeof(postMsg));
    delete postErrFile;

    AssertStrMatch("ThrowError debug sm file", postMsg, TEST_LOG_THROW_ERROR_DEBUG_SM_EXP);
    AssertStrMatch("ThrowErrorEx debug sm file", postMsg, TEST_LOG_THROW_ERROR_EX_DEBUG_SM_EXP);
    AssertStrMatch("ThrowErrorAmxTpl debug sm file", postMsg, TEST_LOG_THROW_ERROR_INFO_SM_EXP);
    AssertStrMatch("ThrowError info sm file", postMsg, TEST_LOG_THROW_ERROR_INFO_SM_EXP);
    AssertStrMatch("ThrowErrorEx info sm file", postMsg, TEST_LOG_THROW_ERROR_EX_INFO_SM_EXP);
    AssertStrMatch("ThrowErrorAmxTpl info sm file", postMsg, TEST_LOG_THROW_ERROR_AMXTPL_INFO_SM_EXP);


#define TEST_LOG_THROW_ERROR_EXP "\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] Exception reported: test message 1\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] Blaming: test-logger-log\\.smx\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] Call stack trace:\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\]   \\[0\\] Logger\\.ThrowError\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\]   \\[1\\] Line [0-9]+, .*test-logger-log\\.sp::Frame_ThorwErrorInfo\\s"

#define TEST_LOG_THROW_ERROR_EX_EXP "\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] Exception reported: test message 2\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] Blaming: test-logger-log\\.smx\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] Call stack trace:\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\]   \\[0\\] Logger\\.ThrowErrorEx\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\]   \\[1\\] Line [0-9]+, .*test-logger-log\\.sp::Frame_ThorwErrorExInfo\\s"

#define TEST_LOG_THROW_ERROR_EX_AMXTPL_EXP "\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] Exception reported: test message 3\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] Blaming: test-logger-log\\.smx\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\] Call stack trace:\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\]   \\[0\\] Logger\\.ThrowErrorAmxTpl\\s\
\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}\\] \\["  ... LOGGER_NAME ... "\\] \\[info\\]   \\[1\\] Line [0-9]+, .*test-logger-log\\.sp::Frame_ThorwErrorAmxTplInfo\\s"

    char path[PLATFORM_MAX_PATH];
    BuildTestPath(path, sizeof(path), "logger-log/throw-error.log");

    AssertFileMatch("ThrowError", path, TEST_LOG_THROW_ERROR_EXP);
    AssertFileMatch("ThrowErrorEx", path, TEST_LOG_THROW_ERROR_EX_EXP);
    AssertFileMatch("ThrowErrorAmxTpl", path, TEST_LOG_THROW_ERROR_EX_AMXTPL_EXP);
}

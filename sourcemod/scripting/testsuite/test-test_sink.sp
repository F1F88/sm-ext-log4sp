#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <log4sp>

#include "test_sink"
#include "test_utils"


public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_test_sink", Command_Test);
}

Action Command_Test(int args)
{
    PrintToServer("---- START TEST TEST SINK ----");

    TestTestSinkCount();

    TestTestSinkDrain();

    TestTestSinkDelay();

    TestTestSinkLogException();

    TestTestSinkFlushException();

    PrintToServer("---- START TEST TEST SINK ----");
    return Plugin_Handled;
}


void TestTestSinkCount()
{
    SetTestContext("Test TestSink Count");

    TestSink sink = new TestSink();
    Logger logger = new Logger("test-sink");
    logger.AddSink(sink);

    logger.Trace("hello test sink 1");
    logger.Debug("hello test sink 2");
    logger.Info("hello test sink 3");
    logger.Warn("hello test sink 4");
    logger.Error("hello test sink 5");
    logger.Fatal("hello test sink 6");
    logger.Log(LogLevel_Off, "hello test sink 7");

    AssertEq("Plain log - log counter", sink.GetLogCount(), 5);
    AssertEq("Plain log - flush counter", sink.GetFlushCount(), 0);

    logger.Flush();

    AssertEq("Flush - log counter", sink.GetLogCount(), 5);
    AssertEq("Flush - flush counter", sink.GetFlushCount(), 1);

    logger.Close();
    sink.Close();
}

void TestTestSinkDrain()
{
    SetTestContext("Test TestSink Drain");

    TestSink sink = new TestSink();
    Logger logger = new Logger("test-sink");
    logger.AddSink(sink);
    logger.SetPattern("%l %v");

    logger.Trace("hello test sink 1");
    logger.Debug("hello test sink 2");
    logger.Info("hello test sink 3");
    logger.Warn("hello test sink 4");
    logger.Error("hello test sink 5");
    logger.Fatal("hello test sink 6");

    AssertEq("GetLogCount", sink.GetLogCount(), 4);
    AssertEq("GetFlushCount", sink.GetFlushCount(), 0);

    logger.Flush();
    AssertEq("GetFlushCount", sink.GetFlushCount(), 1);

    ArrayList logMessages = sink.DrainMsgs();

    sLogMessage logMessage;
    logMessages.GetArray(0, logMessage);
    AssertStrEq("DrainMsgsEx name", logMessage.name, "test-sink");

    logMessages.GetArray(1, logMessage);
    AssertEq("DrainMsgsEx lvl", view_as<int>(logMessage.lvl), LOG4SP_LEVEL_WARN);

    logMessages.GetArray(2, logMessage);
    AssertStrEq("DrainMsgsEx msg", logMessage.msg, "hello test sink 5");

    logMessages.GetArray(3, logMessage);
    AssertStrEq("DrainMsgsEx file", logMessage.file, "");
    delete logMessages;

    AssertStrEq("DrainLastLine 6", sink.DrainLastLine(), LOG4SP_LEVEL_NAME_FATAL ... " hello test sink 6");
    AssertStrEq("DrainLastLine 5", sink.DrainLastLine(), LOG4SP_LEVEL_NAME_ERROR ... " hello test sink 5");
    AssertStrEq("DrainLastLine 4", sink.DrainLastLine(), LOG4SP_LEVEL_NAME_WARN ... " hello test sink 4");
    AssertStrEq("DrainLastLine 3", sink.DrainLastLine(), LOG4SP_LEVEL_NAME_INFO ... " hello test sink 3");

    logger.Close();
    sink.Close();
}

void TestTestSinkDelay()
{
    SetTestContext("Test TestSink Delay");

    const int logDelay = 1001;
    const int flushDelay = 2001;
    int tolerance = RoundToCeil(GetTickInterval() * 1000) * 2;

    TestSink sink = new TestSink();
    Logger logger = new Logger("test-sink");
    logger.AddSink(sink);

    int beforeTime = GetSysTickCount();
    sink.SetLogDelay(logDelay);
    sink.SetFlushDelay(flushDelay);

    logger.Info("hello test sink 1");
    AssertTrue("Log delay >= delay - tolerance", (GetSysTickCount() - beforeTime) >= (logDelay - tolerance));
    AssertTrue("Log delay <= delay + tolerance", (GetSysTickCount() - beforeTime) <= (logDelay + tolerance));

    logger.Flush();
    AssertTrue("Flush delay >= delay - tolerance", (GetSysTickCount() - beforeTime) >= logDelay + flushDelay - tolerance);
    AssertTrue("Flush delay <= delay + tolerance", (GetSysTickCount() - beforeTime) <= logDelay + flushDelay + tolerance);

    logger.Close();
    sink.Close();
}

void TestTestSinkLogException()
{
    SetTestContext("Test TestSink Log Exception");

    char errorDescription[] = "Manually set log exception";
    char name[] = "test-sink";
    const LogLevel lvl = LogLevel_Error;
    char msg[] = "This log message will not be logging.";

    TestSink sink = new TestSink();
    sink.SetLogException(errorDescription);

    char error1[256];
    int bytes1;
    bool result1 = sink.LogTry(name, lvl, msg, .error=error1, .maxlen=sizeof(error1), .bytes=bytes1);
    int  logCnt1 = sink.GetLogCount();

    sink.ClearLogException();

    int bytes2;
    bool result2 = sink.LogTry(name, lvl, msg, .error="", .maxlen=0, .bytes=bytes2);
    int  logCnt2 = sink.GetLogCount();

    sink.Close();

    AssertTrue("LogTry 1 result", result1);
    AssertStrEq("LogTry 1 error description", error1, errorDescription);
    AssertEq("LogTry 1 error description bytes", bytes1, sizeof(errorDescription) - 1);
    AssertEq("LogTry 1 log count", logCnt1, 0);

    AssertFalse("LogTry 2 result", result2);
    AssertEq("LogTry 2 error description bytes", bytes2, 0);
    AssertEq("LogTry 2 log count", logCnt2, 1);
}

void TestTestSinkFlushException()
{
    SetTestContext("Test TestSink Flush Exception");

    char errorDescription[] = "Manually set flush exception";

    TestSink sink = new TestSink();
    sink.SetFlushException(errorDescription);

    char error1[256];
    int bytes1;
    bool result1 = sink.FlushTry(error1, sizeof(error1), bytes1);
    int flushCnt1 = sink.GetFlushCount();

    sink.ClearFlushException();

    int bytes2;
    bool result2 = sink.FlushTry("", 0, bytes2);
    int flushCnt2 = sink.GetFlushCount();

    sink.Close();

    AssertTrue("FlushTry 1 result", result1);
    AssertStrEq("FlushTry 1 error description", error1, errorDescription);
    AssertEq("FlushTry 1 error description bytes", bytes1, sizeof(errorDescription) - 1);
    AssertEq("FlushTry 1 flush count", flushCnt1, 0);

    AssertFalse("FlushTry 2 result", result2);
    AssertEq("FlushTry 2 error description bytes", bytes2, 0);
    AssertEq("FlushTry 2 flush count", flushCnt2, 1);
}

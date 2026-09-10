#pragma semicolon 1
#pragma newdecls required

#if !defined DEBUG
    #define  DEBUG
#endif

#if !defined _DEBUG
    #define  _DEBUG
#endif

#if defined NDEBUG
    #undef  NDEBUG
#endif

#include <sourcemod>
#include <log4sp>

#include "../assert"
#include "../test_sink"
#include "../test_utils"


public void OnPluginStart()
{
    Test();
    RegServerCmd("sm_log4sp_test_test_logger", Command_Test);
}

Action Command_Test(int args)
{
    Test();
    return Plugin_Handled;
}


void Test()
{
    PrintToServer("----------- Started testing Test-Logger ----------");

    TestTestSinkCount();

    TestTestSinkDrain();

    TestTestSinkDelay();

    TestTestSinkLogError();

    TestTestSinkFlushError();

    PrintToServer("------------- Test Test-Logger ended -------------");
}

void TestTestSinkCount()
{
    SetTestContext("TestSink");

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

    AssertEq("[Only log - log counter]", sink.GetLogCount(), 5);
    AssertEq("[Only log - flush counter]", sink.GetFlushCount(), 0);

    logger.Flush();

    AssertEq("[Flush - log counter]", sink.GetLogCount(), 5);
    AssertEq("[Flush - flush counter]", sink.GetFlushCount(), 1);

    sink.Close();
    logger.Close();
}

void TestTestSinkDrain()
{
    SetTestContext("TestSink Drain");

    TestSink sink = new TestSink();
    Logger logger = new Logger("test-sink");
    logger.AddSink(sink);

    logger.Trace("hello test sink 1");
    logger.Debug("hello test sink 2");
    logger.Info("hello test sink 3");
    logger.Warn("hello test sink 4");
    logger.Error("hello test sink 5");
    logger.Fatal("hello test sink 6");

    AssertEq("[Only log - log counter]", sink.GetLogCount(), 4);
    AssertEq("[Only log - flush counter]", sink.GetFlushCount(), 0);

    logger.Flush();
    logger.Close();
    AssertEq("[Flush - flush counter]", sink.GetFlushCount(), 1);

    ArrayList logEvents = sink.Drain();
    sink.Close();
    AssertEq("[LogEvents length]", logEvents.Length, 4);

    LogEvent logEvent;
    logEvents.GetArray(0, logEvent);
    AssertStrEq("[LogEvents name]", logEvent.name, "test-sink");

    logEvents.GetArray(1, logEvent);
    AssertEq("[LogEvents lvl]", view_as<int>(logEvent.lvl), LOG4SP_LEVEL_WARN);

    logEvents.GetArray(2, logEvent);
    AssertStrEq("[LogEvents msg]", logEvent.msg, "hello test sink 5");

    logEvents.GetArray(3, logEvent);
    AssertStrEq("[LogEvents file]", logEvent.loc.filename, "");
    delete logEvents;
}

void TestTestSinkDelay()
{
    SetTestContext("TestSink Delay");

    const int logDelay = 500;
    const int flushDelay = 1000;
    int tolerance = RoundToCeil(GetTickInterval() * 1000) * 2;

    TestSink sink = new TestSink();
    sink.SetLogDelay(logDelay);
    sink.SetFlushDelay(flushDelay);

    SourceLoc loc;
    int beforeTime = GetSysTickCount();

    AssertTrue("Log result", sink.Log(NULL_STRING, loc, "name", LogLevel_Info, "msg"));
    AssertGe("[Log delay >= delay - tolerance]", (GetSysTickCount() - beforeTime), (logDelay - tolerance));
    AssertLe("[Log delay <= delay + tolerance]", (GetSysTickCount() - beforeTime), (logDelay + tolerance));

    AssertTrue("Flush result", sink.Flush());
    AssertGe("[Flush delay >= delay - tolerance]", (GetSysTickCount() - beforeTime), (logDelay + flushDelay - tolerance));
    AssertLe("[Flush delay <= delay + tolerance]", (GetSysTickCount() - beforeTime), (logDelay + flushDelay + tolerance));

    sink.Close();
}

void TestTestSinkLogError()
{
    SetTestContext("TestSink Log Error");

    char errorDescription[] = "Manually set log error";
    char name[] = "test-sink";
    const LogLevel lvl = LogLevel_Error;
    char msg[] = "This log message will not be logging.";

    TestSink sink = new TestSink();
    sink.SetLogError(errorDescription);

    SourceLoc loc;

    char error1[256];
    bool result1 = sink.Log("", loc, name, lvl, msg, .error=error1, .maxlen=sizeof(error1));
    int  logCnt1 = sink.GetLogCount();

    sink.ClearLogError();

    char error2[256];
    bool result2 = sink.Log("", loc, name, lvl, msg, .error=error2, .maxlen=sizeof(error2));
    int  logCnt2 = sink.GetLogCount();

    sink.Close();

    AssertEq("[Log counter]", logCnt1, 0);
    AssertFalse("[Log failed]", result1);
    AssertStrEq("[Log error]", error1, errorDescription);
    AssertEq("[Log error length]", strlen(error1), sizeof(errorDescription) - 1);

    AssertEq("[Log counter]", logCnt2, 1);
    AssertTrue("[Log successful]", result2);
    AssertStrEq("[Log error]", error2, "");
    AssertEq("[Log error length]", strlen(error2), 0);
}

void TestTestSinkFlushError()
{
    SetTestContext("TestSink Flush Error");

    char errorDescription[] = "Manually set flush error";

    TestSink sink = new TestSink();
    sink.SetFlushError(errorDescription);

    char error1[256];
    bool result1 = sink.Flush(error1, sizeof(error1));
    int flushCnt1 = sink.GetFlushCount();

    sink.ClearFlushError();

    char error2[256];
    bool result2 = sink.Flush(error2, sizeof(error2));
    int flushCnt2 = sink.GetFlushCount();

    sink.Close();

    AssertEq("[Flush counter]", flushCnt1, 0);
    AssertFalse("[Flush failed]", result1);
    AssertStrEq("[Flush error]", error1, errorDescription);
    AssertEq("[FlushT error length]", strlen(error1), sizeof(errorDescription) - 1);

    AssertEq("[Flush counter]", flushCnt2, 1);
    AssertTrue("[Flush successful]", result2);
    AssertStrEq("[Flush error]", error2, "");
    AssertEq("[Flush error length]", strlen(error2), 0);
}

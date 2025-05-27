#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <testing>
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

    delete logger;
    delete sink;
}

void TestTestSinkDrain()
{
    SetTestContext("Test TestSink Drain");

    TestSink sink = new TestSink();
    Logger logger = new Logger("test-sink");
    logger.AddSink(sink);

    logger.Trace("hello test sink 1");
    logger.Debug("hello test sink 2");
    logger.Info("hello test sink 3");
    logger.Warn("hello test sink 4");
    logger.Error("hello test sink 5");

    logger.SetPattern("%v");
    logger.Fatal("hello test sink 6");

    sink.DrainLastMsg(CB_DarinLastMsg, 7);
    sink.DrainLastLine(CB_DarinLastLine, 8);
    sink.DrainLastLine(CB_DarinLastLine2, 9);

    delete logger;
    delete sink;
}

static void CB_DarinLastMsg(const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int logTime, any data)
{
    AssertStrEq("DrainLastMsg name", name, "test-sink");
    AssertEq("DrainLastMsg lvl", lvl, LogLevel_Fatal);
    AssertStrEq("DrainLastMsg msg", msg, "hello test sink 6");
    AssertEq("DrainLastMsg data", data, 7);
}

static void CB_DarinLastLine(const char[] msg, any data)
{
    AssertStrEq("DrainLastLine msg", msg, "hello test sink 6");
    AssertEq("DrainLastLine data", data, 8);
}

static void CB_DarinLastLine2(const char[] msg, any data)
{
    AssertStrMatch("DrainLastLine msg match", msg, P_PREFIX ... "hello test sink 5");
    AssertEq("DrainLastLine data", data, 9);
}

void TestTestSinkDelay()
{
    SetTestContext("Test TestSink Delay");

    TestSink sink = new TestSink();
    Logger logger = new Logger("test-sink");
    logger.AddSink(sink);

    int beforeTime = GetTime();
    sink.SetLogDelay(2001);
    sink.SetFlushDelay(2001);

    logger.Info("hello test sink 1");
    AssertTrue("Log delay >= 2", (GetTime() - beforeTime) >= 2);

    logger.Flush();
    AssertTrue("Flush delay >= 4", (GetTime() - beforeTime) >= 4);

    delete logger;
    delete sink;
}

void TestTestSinkLogException()
{
    SetTestContext("Test TestSink Log Exception");

    TestSink sink = new TestSink();
    Logger logger = new Logger("test-sink");
    logger.AddSink(sink);

    sink.SetLogException("Manually set log exceptions");

    MarkErrorTestStart("Test TestSink Log Exception");
    logger.Info("hello test sink 1");
    MarkErrorTestEnd("Test TestSink Log Exception");

    AssertEq("Log cnt", sink.GetLogCount(), 0);

    sink.ClearLogException();
    logger.Info("hello test sink 2");
    AssertEq("Log cnt", sink.GetLogCount(), 1);

    delete logger;
    delete sink;

    AssertEq("SM file line cnt", CountLines(GetErrorFilename()), 3);
    DeleteFile(GetErrorFilename());
}

void TestTestSinkFlushException()
{
    SetTestContext("Test TestSink Flush Exception");

    TestSink sink = new TestSink();
    Logger logger = new Logger("test-sink");
    logger.AddSink(sink);

    sink.SetFlushException("Manually set flush exceptions");

    MarkErrorTestStart("Test TestSink Flush Exception");
    logger.Flush();
    MarkErrorTestEnd("Test TestSink Flush Exception");

    AssertEq("Flush cnt", sink.GetFlushCount(), 0);

    sink.ClearFlushException();
    logger.Flush();
    AssertEq("Flush cnt", sink.GetFlushCount(), 1);

    delete logger;
    delete sink;

    AssertEq("SM file line cnt", CountLines(GetErrorFilename()), 3);
    DeleteFile(GetErrorFilename());
}

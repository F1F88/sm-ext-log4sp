#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <log4sp>

#include "test_sink"


public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_log_level", Command_Test);
}

Action Command_Test(int args)
{
    PrintToServer("---- START TEST LOG LEVEL ----");

    TestDefaultLevel();

    TestLoggerLevels();

    TestSinkLevels();

    PrintToServer("---- STOP TEST LOG LEVEL ----");
    return Plugin_Handled;
}


void TestDefaultLevel()
{
    SetTestContext("Test Default Log Level");

    TestSink sink = new TestSink();
    AssertEq("sink lvl", sink.GetLevel(), LogLevel_Trace);

    Logger logger = new Logger("test-level");
    AssertEq("logger lvl", logger.GetLevel(), LogLevel_Info);

    logger.Close();
    sink.Close();
}


void TestLoggerLevels()
{
    SetTestContext("Test Logger Log Levels");

    for (int i = 0; i < TEST_LOG4SP_LEVEL_TOTAL; ++i)
    {
        TestLoggerLevel(view_as<LogLevel>(i));
    }
}

// test that logger log only messages with level bigger or equal to its level
void TestLoggerLevel(LogLevel level)
{
    TestSink sink = new TestSink();
    Logger logger = new Logger("test-level");
    logger.AddSink(sink);

    AssertEq("Set logger lvl pre", logger.GetLevel(), LogLevel_Info);
    logger.SetLevel(level);
    AssertEq("Set logger lvl post", logger.GetLevel(), level);

    AssertEq("Sink lvl keep", sink.GetLevel(), LogLevel_Trace);

    logger.SetPattern("%l %v");

    logger.Trace("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_TRACE)
        AssertStrMatch("trace", sink.GetLastLogLine(), "trace hello\\s");
    else
        AssertStrEq("trace", sink.GetLastLogLine(), "");

    logger.Debug("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_DEBUG)
        AssertStrMatch("debug", sink.GetLastLogLine(), "debug hello\\s");
    else
        AssertStrEq("debug", sink.GetLastLogLine(), "");

    logger.Info("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_INFO)
        AssertStrMatch("info", sink.GetLastLogLine(), "info hello\\s");
    else
        AssertStrEq("info", sink.GetLastLogLine(), "");

    logger.Warn("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_WARN)
        AssertStrMatch("warn", sink.GetLastLogLine(), "warn hello\\s");
    else
        AssertStrEq("warn", sink.GetLastLogLine(), "");

    logger.Error("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_ERROR)
        AssertStrMatch("error", sink.GetLastLogLine(), "error hello\\s");
    else
        AssertStrEq("error", sink.GetLastLogLine(), "");

    logger.Fatal("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_FATAL)
        AssertStrMatch("fatal", sink.GetLastLogLine(), "fatal hello\\s");
    else
        AssertStrEq("fatal", sink.GetLastLogLine(), "");

    logger.Close();
    sink.Close();
}


void TestSinkLevels()
{
    SetTestContext("Test Sink Log Levels");

    for (int i = 0; i < TEST_LOG4SP_LEVEL_TOTAL; ++i)
    {
        TestSinkLevel(view_as<LogLevel>(i));
    }
}

// test that sink displays all messages with level bigger or equal to its level
void TestSinkLevel(LogLevel level)
{
    TestSink sink = new TestSink();
    Logger logger = new Logger("test-level");
    logger.AddSink(sink);

    AssertEq("Set logger lvl pre", logger.GetLevel(), LogLevel_Info);
    logger.SetLevel(level);
    AssertEq("Set logger lvl post", logger.GetLevel(), level);

    AssertEq("Set sink lvl pre", sink.GetLevel(), LogLevel_Trace);
    sink.SetLevel(level);
    AssertEq("Set sink lvl post", sink.GetLevel(), level);

    logger.SetPattern("%l %v");

    logger.Trace("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_TRACE)
        AssertStrMatch("trace", sink.GetLastLogLine(), "trace hello\\s");
    else
        AssertStrEq("trace", sink.GetLastLogLine(), "");

    logger.Debug("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_DEBUG)
        AssertStrMatch("debug", sink.GetLastLogLine(), "debug hello\\s");
    else
        AssertStrEq("debug", sink.GetLastLogLine(), "");

    logger.Info("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_INFO)
        AssertStrMatch("info", sink.GetLastLogLine(), "info hello\\s");
    else
        AssertStrEq("info", sink.GetLastLogLine(), "");

    logger.Warn("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_WARN)
        AssertStrMatch("warn", sink.GetLastLogLine(), "warn hello\\s");
    else
        AssertStrEq("warn", sink.GetLastLogLine(), "");

    logger.Error("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_ERROR)
        AssertStrMatch("error", sink.GetLastLogLine(), "error hello\\s");
    else
        AssertStrEq("error", sink.GetLastLogLine(), "");

    logger.Fatal("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_FATAL)
        AssertStrMatch("fatal", sink.GetLastLogLine(), "fatal hello\\s");
    else
        AssertStrEq("fatal", sink.GetLastLogLine(), "");

    logger.Close();
    sink.Close();
}

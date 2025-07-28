#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <log4sp>

#include "test_sink"
#include "test_utils"


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
    AssertEq("Sink level", view_as<int>(sink.GetLevel()), LOG4SP_LEVEL_TRACE);

    Logger logger = new Logger("test-level");
    AssertEq("Logger level", view_as<int>(logger.GetLevel()), LOG4SP_LEVEL_INFO);

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

    AssertEq("Before set logger level, logger level", view_as<int>(logger.GetLevel()), LOG4SP_LEVEL_INFO);
    logger.SetLevel(level);
    AssertEq("After set logger level, logger level", view_as<int>(logger.GetLevel()), view_as<int>(level));

    AssertEq("After set logger level, sink level keep", view_as<int>(sink.GetLevel()), LOG4SP_LEVEL_TRACE);

    logger.SetPattern("%l %v");

    logger.Trace("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_TRACE)
        AssertStrEq("Trace message", sink.DrainLastLine(), "trace hello");

    logger.Debug("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_DEBUG)
        AssertStrEq("Debug message", sink.DrainLastLine(), "debug hello");

    logger.Info("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_INFO)
        AssertStrEq("Info message", sink.DrainLastLine(), "info hello");

    logger.Warn("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_WARN)
        AssertStrEq("Warn message", sink.DrainLastLine(), "warn hello");

    logger.Error("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_ERROR)
        AssertStrEq("Error message", sink.DrainLastLine(), "error hello");

    logger.Fatal("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_FATAL)
        AssertStrEq("Fatal message", sink.DrainLastLine(), "fatal hello");

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

    AssertEq("Before set logger level, logger level", view_as<int>(logger.GetLevel()), LOG4SP_LEVEL_INFO);
    logger.SetLevel(level);
    AssertEq("After set logger level, logger level", view_as<int>(logger.GetLevel()), view_as<int>(level));

    AssertEq("Before set sink level, sink level", view_as<int>(sink.GetLevel()), LOG4SP_LEVEL_TRACE);
    sink.SetLevel(level);
    AssertEq("Before set sink level, sink level", view_as<int>(sink.GetLevel()), view_as<int>(level));

    logger.SetPattern("%l %v");

    logger.Trace("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_TRACE)
        AssertStrEq("Trace message", sink.DrainLastLine(), "trace hello");

    logger.Debug("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_DEBUG)
        AssertStrEq("Debug message", sink.DrainLastLine(), "debug hello");

    logger.Info("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_INFO)
        AssertStrEq("Info message", sink.DrainLastLine(), "info hello");

    logger.Warn("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_WARN)
        AssertStrEq("Warn message", sink.DrainLastLine(), "warn hello");

    logger.Error("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_ERROR)
        AssertStrEq("Error message", sink.DrainLastLine(), "error hello");

    logger.Fatal("hello");
    if (view_as<int>(level) <= LOG4SP_LEVEL_FATAL)
        AssertStrEq("Fatal message", sink.DrainLastLine(), "fatal hello");

    logger.Close();
    sink.Close();
}

#include <log4sp>

#include "test_sink"

#pragma semicolon 1
#pragma newdecls required


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

    TestSink sink = TestSink.Initialize();
    AssertEq("sink lvl", sink.GetLevel(), LogLevel_Trace);

    Logger logger = new Logger("test-level");
    AssertEq("logger lvl", logger.GetLevel(), LogLevel_Info);

    delete logger;
    TestSink.Destroy();
}


void TestLoggerLevels()
{
    SetTestContext("Test Logger Log Levels");

    for (int i = 0; i < LOG4SP_LEVEL_OFF + 1; ++i)
    {
        TestLoggerLevel(view_as<LogLevel>(i));
    }
}

// test that logger log only messages with level bigger or equal to its level
void TestLoggerLevel(LogLevel level)
{
    TestSink sink = TestSink.Initialize();
    Logger logger = new Logger("test-level");
    logger.AddSink(sink);

    AssertEq("Set logger lvl pre", logger.GetLevel(), LogLevel_Info);
    logger.SetLevel(level);
    AssertEq("Set logger lvl post", logger.GetLevel(), level);

    AssertEq("Sink lvl keep", sink.GetLevel(), LogLevel_Trace);

    logger.SetPattern("%l %v");

    logger.Trace("hello");
    logger.Debug("hello");
    logger.Info("hello");
    logger.Warn("hello");
    logger.Error("hello");
    logger.Fatal("hello");

    ArrayList expected = GetExpected(level);

    TestSink.AssertLinesRegex("Log post lines", sink.GetLines(), expected);

    delete logger;
    TestSink.Destroy();
    delete expected;
}


void TestSinkLevels()
{
    SetTestContext("Test Sink Log Levels");

    for (int i = 0; i < LOG4SP_LEVEL_OFF + 1; ++i)
    {
        TestSinkLevel(view_as<LogLevel>(i));
    }
}

// test that sink displays all messages with level bigger or equal to its level
void TestSinkLevel(LogLevel level)
{
    TestSink sink = TestSink.Initialize();
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
    logger.Debug("hello");
    logger.Info("hello");
    logger.Warn("hello");
    logger.Error("hello");
    logger.Fatal("hello");

    ArrayList expected = GetExpected(level);

    TestSink.AssertLinesRegex("Log post lines", sink.GetLines(), expected);

    delete logger;
    TestSink.Destroy();
    delete expected;
}


ArrayList GetExpected(LogLevel level)
{
    // 只在错误时输出
    if (view_as<int>(level) < 0 || level >= LogLevel_Total)
        AssertTrue("(0 <= level < LogLevel_Total)", 0 <= view_as<int>(level) && level < LogLevel_Total);

    ArrayList expected = new ArrayList(ByteCountToCells(TEST_MAX_MSG_LENGTH));

    if (level <= LogLevel_Trace)
    {
        expected.PushString("trace hello\\s");
    }

    if (level <= LogLevel_Debug)
    {
        expected.PushString("debug hello\\s");
    }

    if (level <= LogLevel_Info)
    {
        expected.PushString("info hello\\s");
    }

    if (level <= LogLevel_Warn)
    {
        expected.PushString("warn hello\\s");
    }

    if (level <= LogLevel_Error)
    {
        expected.PushString("error hello\\s");
    }

    if (level <= LogLevel_Fatal)
    {
        expected.PushString("fatal hello\\s");
    }

    return expected;
}

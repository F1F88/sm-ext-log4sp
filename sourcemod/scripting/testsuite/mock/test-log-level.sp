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
    RegServerCmd("sm_log4sp_test_log_level", Command_Test);
}

Action Command_Test(int args)
{
    Test();
    return Plugin_Handled;
}


void Test()
{
    PrintToServer("----------- Started testing Log-Level -----------");

    TestDefaultLevel();

    TestModifyLogLevels();

    TestModifyThrowLevels();

    PrintToServer("-------------- Test Log-Level ended -------------");
}

void TestDefaultLevel()
{
    SetTestContext("LogLevel Default");

    TestSink sink = new TestSink();
    LogLevel sinkLevel = sink.GetLevel();
    SinkCloseAndDelete(sink);

    Logger logger = new Logger("test-level");
    LogLevel loggerLevel = logger.GetLevel();
    LogLevel loggerFlushLevel = logger.GetFlushLevel();
    LogLevel loggerThrowLevel = logger.GetThrowLevel();
    LoggerCloseAndDelete(logger);

    AssertEq("[Sink log]",      sinkLevel,          LogLevel_Trace);
    AssertEq("[Logger log]",    loggerLevel,        LogLevel_Info);
    AssertEq("[Logger flush]",  loggerFlushLevel,   LogLevel_Off);
    AssertEq("[Logger throw]",  loggerThrowLevel,   LogLevel_Off);
}

void TestModifyLogLevels()
{
    SetTestContext("Modify LogLevels");
    for (int i = 0; i < view_as<int>(LogLevel_Total); ++i)
    {
        TestModifyLogLevel(view_as<LogLevel>(i));
    }
}

void TestModifyLogLevel(LogLevel level)
{
    TestSink sink = new TestSink();
    Logger logger = new Logger("test-log-level");
    logger.AddSink(sink);

    AssertEq("[Before logger.SetLevel]", logger.GetLevel(), LogLevel_Info);
    logger.SetLevel(level);
    AssertEq("[After  logger.SetLevel]", logger.GetLevel(), level);

    AssertEq("[Before sink.SetLevel]", sink.GetLevel(), LogLevel_Trace);
    sink.SetLevel(level);
    AssertEq("[After  sink.SetLevel]", sink.GetLevel(), level);

    logger.Trace("hello");
    if (level <= LogLevel_Trace)
        AssertEq("[Trace]", sink.DrainLatest().lvl, LogLevel_Trace);

    logger.Debug("hello");
    if (level <= LogLevel_Debug)
        AssertEq("[Debug]", sink.DrainLatest().lvl, LogLevel_Debug);

    logger.Info("hello");
    if (level <= LogLevel_Info)
        AssertEq("[Info ]", sink.DrainLatest().lvl, LogLevel_Info);

    logger.Warn("hello");
    if (level <= LogLevel_Warn)
        AssertEq("[Warn ]", sink.DrainLatest().lvl, LogLevel_Warn);

    logger.Error("hello");
    if (level <= LogLevel_Error)
        AssertEq("[Error]", sink.DrainLatest().lvl, LogLevel_Error);

    logger.Fatal("hello");
    if (level <= LogLevel_Fatal)
        AssertEq("[Fatal]", sink.DrainLatest().lvl, LogLevel_Fatal);

    SinkCloseAndDelete(sink);
    LoggerCloseAndDelete(logger);
}

void TestModifyThrowLevels()
{
    SetTestContext("Modify ThrowLevels");
    for (int i = 0; i < view_as<int>(LogLevel_Total); ++i)
    {
        TestModifyThrowLevel(view_as<LogLevel>(i));
    }
}

void TestModifyThrowLevel(LogLevel level)
{
    Logger logger = new Logger("test-throw-level");

    AssertEq("[Before logger.SetThrowLevel]", logger.GetThrowLevel(), LogLevel_Off);
    logger.SetThrowLevel(level);
    AssertEq("[After  logger.SetThrowLevel]", logger.GetThrowLevel(), level);

    if (level <= LogLevel_Trace)
        AssertTrue("[Trace]", logger.ShouldThrow(LogLevel_Trace));

    if (level <= LogLevel_Debug)
        AssertTrue("[Debug]", logger.ShouldThrow(LogLevel_Debug));

    if (level <= LogLevel_Info)
        AssertTrue("[Info ]", logger.ShouldThrow(LogLevel_Info));

    if (level <= LogLevel_Warn)
        AssertTrue("[Warn ]", logger.ShouldThrow(LogLevel_Warn));

    if (level <= LogLevel_Error)
        AssertTrue("[Error]", logger.ShouldThrow(LogLevel_Error));

    if (level <= LogLevel_Fatal)
        AssertTrue("[Fatal]", logger.ShouldThrow(LogLevel_Fatal));

    if (level <= LogLevel_Off)
        AssertFalse("[Off  ]", logger.ShouldThrow(LogLevel_Off));

    LoggerCloseAndDelete(logger);
}

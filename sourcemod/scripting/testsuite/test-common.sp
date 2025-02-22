#include <sourcemod>

#include <log4sp>

#include "test_utils"

#pragma semicolon 1
#pragma newdecls required

static const char g_expectedLevelNames[][] = {LOG4SP_LEVEL_NAME_TRACE, LOG4SP_LEVEL_NAME_DEBUG,
                                              LOG4SP_LEVEL_NAME_INFO,  LOG4SP_LEVEL_NAME_WARN,
                                              LOG4SP_LEVEL_NAME_ERROR, LOG4SP_LEVEL_NAME_FATAL,
                                              LOG4SP_LEVEL_NAME_OFF};

static const char g_expectedLevelShortNames[][] = {LOG4SP_LEVEL_SHORT_NAME_TRACE, LOG4SP_LEVEL_SHORT_NAME_DEBUG,
                                                   LOG4SP_LEVEL_SHORT_NAME_INFO,  LOG4SP_LEVEL_SHORT_NAME_WARN,
                                                   LOG4SP_LEVEL_SHORT_NAME_ERROR, LOG4SP_LEVEL_SHORT_NAME_FATAL,
                                                   LOG4SP_LEVEL_SHORT_NAME_OFF};

public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_common", Command_Test);
}

Action Command_Test(int args)
{
    PrintToServer("---- START COMMON ----");

    TestCommon();

    PrintToServer("---- STOP COMMON ----");
    return Plugin_Handled;
}


void TestCommon()
{
    SetTestContext("Test Common");

    Logger logger = ServerConsoleSink.CreateLogger("test-common");

    char level[TEST_LOG4SP_LEVEL_TOTAL][64];
    char levelShort[TEST_LOG4SP_LEVEL_TOTAL][64];

    for (int i = 0; i < TEST_LOG4SP_LEVEL_TOTAL; ++i)
    {
        LogLevelToName(level[i], sizeof(level[]), view_as<LogLevel>(i));
        LogLevelToShortName(levelShort[i], sizeof(levelShort[]), view_as<LogLevel>(i));
    }

    AssertStrArrayEq("Log level name", level, g_expectedLevelNames, TEST_LOG4SP_LEVEL_TOTAL);
    AssertStrArrayEq("Log level short name", levelShort, g_expectedLevelShortNames, TEST_LOG4SP_LEVEL_TOTAL);

    AssertEq("Name '" ... LOG4SP_LEVEL_NAME_TRACE ... "' to lvl", NameToLogLevel(LOG4SP_LEVEL_NAME_TRACE), LogLevel_Trace);
    AssertEq("Name '" ... LOG4SP_LEVEL_NAME_DEBUG ... "' to lvl", NameToLogLevel(LOG4SP_LEVEL_NAME_DEBUG), LogLevel_Debug);
    AssertEq("Name '" ... LOG4SP_LEVEL_NAME_INFO  ... "' to lvl", NameToLogLevel(LOG4SP_LEVEL_NAME_INFO),  LogLevel_Info);
    AssertEq("Name '" ... LOG4SP_LEVEL_NAME_WARN  ... "' to lvl", NameToLogLevel(LOG4SP_LEVEL_NAME_WARN),  LogLevel_Warn);
    AssertEq("Name '" ... LOG4SP_LEVEL_NAME_ERROR ... "' to lvl", NameToLogLevel(LOG4SP_LEVEL_NAME_ERROR), LogLevel_Error);
    AssertEq("Name '" ... LOG4SP_LEVEL_NAME_FATAL ... "' to lvl", NameToLogLevel(LOG4SP_LEVEL_NAME_FATAL), LogLevel_Fatal);
    AssertEq("Name '" ... LOG4SP_LEVEL_NAME_OFF   ... "' to lvl", NameToLogLevel(LOG4SP_LEVEL_NAME_OFF),   LogLevel_Off);
    AssertEq("Name '' to lvl", NameToLogLevel(""), LogLevel_Off);
    AssertEq("Name 'some string' to lvl", NameToLogLevel("some string"), LogLevel_Off);

    SourceLoc loc;
    AssertTrue("Empty source location", loc.IsEmpty());

    delete logger;
}

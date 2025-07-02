#pragma semicolon 1
#pragma newdecls required

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

    char level[64];
    char levelShort[64];

    for (int i = 0; i < TEST_LOG4SP_LEVEL_TOTAL; ++i)
    {
        LogLevelToName(level, sizeof(level), view_as<LogLevel>(i));
        AssertStrEq("Log level name", level, g_expectedLevelNames[i]);

        LogLevelToShortName(levelShort, sizeof(levelShort), view_as<LogLevel>(i));
        AssertStrEq("Log level short name", levelShort, g_expectedLevelShortNames[i]);
    }

    AssertEq("Name '" ... LOG4SP_LEVEL_NAME_TRACE ... "' to lvl", view_as<int>(NameToLogLevel(LOG4SP_LEVEL_NAME_TRACE)), LOG4SP_LEVEL_TRACE);
    AssertEq("Name '" ... LOG4SP_LEVEL_NAME_DEBUG ... "' to lvl", view_as<int>(NameToLogLevel(LOG4SP_LEVEL_NAME_DEBUG)), LOG4SP_LEVEL_DEBUG);
    AssertEq("Name '" ... LOG4SP_LEVEL_NAME_INFO  ... "' to lvl", view_as<int>(NameToLogLevel(LOG4SP_LEVEL_NAME_INFO)),  LOG4SP_LEVEL_INFO);
    AssertEq("Name '" ... LOG4SP_LEVEL_NAME_WARN  ... "' to lvl", view_as<int>(NameToLogLevel(LOG4SP_LEVEL_NAME_WARN)),  LOG4SP_LEVEL_WARN);
    AssertEq("Name '" ... LOG4SP_LEVEL_NAME_ERROR ... "' to lvl", view_as<int>(NameToLogLevel(LOG4SP_LEVEL_NAME_ERROR)), LOG4SP_LEVEL_ERROR);
    AssertEq("Name '" ... LOG4SP_LEVEL_NAME_FATAL ... "' to lvl", view_as<int>(NameToLogLevel(LOG4SP_LEVEL_NAME_FATAL)), LOG4SP_LEVEL_FATAL);
    AssertEq("Name '" ... LOG4SP_LEVEL_NAME_OFF   ... "' to lvl", view_as<int>(NameToLogLevel(LOG4SP_LEVEL_NAME_OFF)),   LOG4SP_LEVEL_OFF);
    AssertEq("Name '' to lvl", view_as<int>(NameToLogLevel("")), LOG4SP_LEVEL_OFF);
    AssertEq("Name 'some string' to lvl", view_as<int>(NameToLogLevel("some string")), LOG4SP_LEVEL_OFF);

    SourceLoc loc;
    AssertTrue("Empty source location", loc.IsEmpty());

    delete logger;
}

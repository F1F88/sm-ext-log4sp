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

#include <log4sp/common>

#include "../assert"
#include "../test_utils"


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
    Test();
    RegServerCmd("sm_log4sp_test_common", Command_Test);
}

Action Command_Test(int args)
{
    Test();
    return Plugin_Handled;
}


void Test()
{
    PrintToServer("------------- Started testing Common -------------");

    TestLogLevelToName();
    TestLogLevelToShortName();
    TestNameToLogLevel();
    TestSourceLoc();

    PrintToServer("---------------- Test Common ended ---------------");
}

void TestLogLevelToName()
{
    SetTestContext("Common LogLevelToName");
    for (int i = 0; i < view_as<int>(LogLevel_Total); ++i)
    {
        char integerStr[12];
        FormatEx(integerStr, sizeof(integerStr), "[%u]", i);

        char levelName[64];
        LogLevelToName(levelName, sizeof(levelName), view_as<LogLevel>(i));
        AssertStrEq(integerStr, levelName, g_expectedLevelNames[i]);
    }
}

void TestLogLevelToShortName()
{
    SetTestContext("Common LogLevelToShortName");
    for (int i = 0; i < view_as<int>(LogLevel_Total); ++i)
    {
        char integerStr[12];
        FormatEx(integerStr, sizeof(integerStr), "[%u]", i);

        char levelShortName[64];
        LogLevelToShortName(levelShortName, sizeof(levelShortName), view_as<LogLevel>(i));
        AssertStrEq(integerStr, levelShortName, g_expectedLevelShortNames[i]);
    }
}

void TestNameToLogLevel()
{
    SetTestContext("Common NameToLogLevel");
    AssertEq("[\"" ... LOG4SP_LEVEL_NAME_TRACE ... "\"]", NameToLogLevel(LOG4SP_LEVEL_NAME_TRACE), LogLevel_Trace);
    AssertEq("[\"" ... LOG4SP_LEVEL_NAME_DEBUG ... "\"]", NameToLogLevel(LOG4SP_LEVEL_NAME_DEBUG), LogLevel_Debug);
    AssertEq("[\"" ... LOG4SP_LEVEL_NAME_INFO  ... "\"]", NameToLogLevel(LOG4SP_LEVEL_NAME_INFO),  LogLevel_Info);
    AssertEq("[\"" ... LOG4SP_LEVEL_NAME_WARN  ... "\"]", NameToLogLevel(LOG4SP_LEVEL_NAME_WARN),  LogLevel_Warn);
    AssertEq("[\"" ... LOG4SP_LEVEL_NAME_ERROR ... "\"]", NameToLogLevel(LOG4SP_LEVEL_NAME_ERROR), LogLevel_Error);
    AssertEq("[\"" ... LOG4SP_LEVEL_NAME_FATAL ... "\"]", NameToLogLevel(LOG4SP_LEVEL_NAME_FATAL), LogLevel_Fatal);
    AssertEq("[\"" ... LOG4SP_LEVEL_NAME_OFF   ... "\"]", NameToLogLevel(LOG4SP_LEVEL_NAME_OFF),   LogLevel_Off);
    AssertEq("[\"\"]",                                    NameToLogLevel(""),                      LogLevel_Off);
    AssertEq("[\"some string\"]",                         NameToLogLevel("some string"),           LogLevel_Off);
}

void TestSourceLoc()
{
    SetTestContext("Common SourceLoc");
    SourceLoc variable1;
    AssertTrue("[variable 1 is empty]", variable1.IsEmpty());

    SourceLoc variable2 = {__BINARY_PATH__, __LINE__, __BINARY_NAME__};
    AssertFalse("[variable 2 is not empty]", variable2.IsEmpty());
    AssertEq("[variable 2 line]", variable2.line, (__LINE__ - 2));
    AssertStrEq("[variable 2 filename]", variable2.filename, __BINARY_PATH__);
    AssertStrEq("[variable 2 funcname]", variable2.funcname, __BINARY_NAME__);

    char buildTime[256], buildTags[256];
    int version = GetLog4spVersion(buildTime, sizeof(buildTime), buildTags, sizeof(buildTags));
    AssertEq("[version major]", (version >> 16) & 0xFF, 1);
    AssertEq("[version minor]", (version >>  8) & 0xFF, 11);
    AssertEq("[version patch]", (version >>  0) & 0xFF, 0);
}

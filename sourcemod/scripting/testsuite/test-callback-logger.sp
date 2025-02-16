#include <log4sp>

#include "test_sink"

#pragma semicolon 1
#pragma newdecls required


public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_callback_logger", Command_Test);
}

Action Command_Test(int args)
{
    PrintToServer("---- START TEST CALLBACK LOGGER ----");

    TestCustomCallbackLogger();

    PrintToServer("---- STOP TEST CALLBACK LOGGER ----");
    return Plugin_Handled;
}


CallbackSink g_hCBSink = null;
ArrayList g_hLogMsgs = null;
ArrayList g_hLogLines = null;

int g_iLogMsgsCounter = 0;
int g_iLogLinesCounter = 0;
int g_iFlushCounter = 0;

void TestCustomCallbackLogger()
{
    SetTestContext("Test Custom Callback Logger");

    g_hLogMsgs = new ArrayList(ByteCountToCells(TEST_MAX_MSG_LENGTH));
    g_hLogLines = new ArrayList(ByteCountToCells(TEST_MAX_MSG_LENGTH));
    g_iLogMsgsCounter = 0;
    g_iLogLinesCounter = 0;
    g_iFlushCounter = 0;

    g_hCBSink = new CallbackSink(CBSink_OnLog, CBSink_OnLogPost, CBSink_OnFlush);
    g_hCBSink.SetFlushCallback(CBSink_OnFlush);

    TestSink testSink = TestSink.Initialize();

    Logger logger = new Logger("test-callback");
    logger.AddSink(g_hCBSink);
    logger.AddSink(testSink);
    logger.SetPattern("[%Y-%m-%d %H:%M:%S] [%n] [%l] [%s:%#] %v");

    for (int i = 0; i < LOG4SP_LEVEL_OFF + 1; ++i)
    {
        logger.LogAmxTpl(view_as<LogLevel>(i), "test message %d - 1", i);
        logger.LogSrcAmxTpl(view_as<LogLevel>(i), "test message %d - 2", i);
        logger.LogLocAmxTpl(__BINARY_PATH__, __LINE__, __BINARY_NAME__, view_as<LogLevel>(i), "test message %d - 3", i);
        // logger.Flush();
    }

    AssertEq("On log msgs counter",             g_iLogMsgsCounter,  testSink.GetLogCount());
    AssertEq("On log post lines counter",       g_iLogLinesCounter, testSink.GetLogPostCount());
    AssertEq("On flush counter",                g_iFlushCounter,    testSink.GeFlushCount());

    TestSink.AssertMsgs("On log msgs",          g_hLogMsgs,         testSink.GetLines());
    TestSink.AssertLines("On log post lines",   g_hLogLines,        testSink.GetLines());

    delete g_hLogMsgs;
    delete g_hLogLines;

    delete logger;
    delete g_hCBSink;
    TestSink.Destroy();
}

void CBSink_OnLog(const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int timePoint)
{
    char buffer[TEST_MAX_MSG_LENGTH];
    g_hCBSink.ToPattern(buffer, sizeof(buffer), name, lvl, msg, file, line, func, timePoint);

    g_hLogMsgs.PushString(buffer);
    g_iLogMsgsCounter++;
}

void CBSink_OnLogPost(const char[] msg)
{
    char buffer[TEST_MAX_MSG_LENGTH];
    strcopy(buffer, sizeof(buffer), msg);

    g_hLogLines.PushString(buffer);
    g_iLogLinesCounter++;
}

void CBSink_OnFlush()
{
    g_iFlushCounter++;
}

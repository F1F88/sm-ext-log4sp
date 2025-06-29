#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <log4sp>

#include "test_sink"
#include "test_utils"


#define LOGGER_NAME     "test-callback"


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


TestSink g_testSink;

void TestCustomCallbackLogger()
{
    SetTestContext("Test Custom Callback Logger");

    // 基于 CallbackSink
    g_testSink = new TestSink();
    CallbackSink sink = new CallbackSink(CBSink_OnLog, CBSink_OnLogPost);

    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(g_testSink);
    logger.AddSink(sink);
    logger.SetPattern("'%Y-%m-%d' '%l' '%n' '%v'");

    for (int i = 0; i < TEST_LOG4SP_LEVEL_TOTAL; ++i)
    {
        logger.LogAmxTpl(view_as<LogLevel>(i), "test message %d", i);
    }

    logger.Flush();

    int logCnt      = g_testSink.GetLogCount();
    int flushCnt    = g_testSink.GetFlushCount();

    g_testSink.Close();
    sink.Close();
    logger.Close();

    //  total - 2 (trace、debug)
    AssertEq("Log count", logCnt, 5);
    AssertEq("Flush count", flushCnt, 1);
}

void CBSink_OnLog(const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int timePoint)
{
    AssertStrEq("OnLog name", name, LOGGER_NAME);
    AssertEq("OnLog lvl", lvl, view_as<LogLevel>(g_testSink.GetLogCount() + 1));
    AssertStrMatch("OnLog msg match", msg, "test message [0-9]");
}

void CBSink_OnLogPost(const char[] msg)
{
    char[] pattern = "'[0-9]{4}-[0-9]{2}-[0-9]{2}' '(info|warn|error|fatal|off)' 'test-callback' 'test message [0-9]'(\n|\r\n)";
    AssertStrMatch("OnLogPost msg match", msg, pattern);
}

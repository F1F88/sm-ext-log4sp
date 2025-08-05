#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <log4sp>

#include "test_utils"


#define LOGGER_NAME     "test-callback2"


int g_iLogCount;
int g_iLogPostCount;
int g_iFlushCount;
int g_iErrorcount;
int g_iDestoryCount;

public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_callback2", Command_Test);
}

Action Command_Test(int args)
{
    PrintToServer("---- START TEST CALLBACK2 LOGGER ----");

    TestCustomCallback2();

    PrintToServer("---- STOP TEST CALLBACK2 LOGGER ----");
    return Plugin_Handled;
}


void TestCustomCallback2()
{
    InitializeGlobal();

    SetTestContext("Test Custom Callback2");

    Logger logger = CallbackSink2.CreateLogger(LOGGER_NAME, CBSink_OnLog, CBSink_OnLogPost, CBSink_OnFlush, CBSink_OnDestory, 7);
    logger.AddSinkEx(new CallbackSink2(CBSink_OnLog2, CBSink_OnLogPost2, CBSink_OnFlush2, _, 9));
    logger.SetPattern("'%Y-%m-%d' '%l' '%n' '%v'");
    logger.SetErrorHandler(CBLogger_ErrorHandler);

    for (int i = 0; i < TEST_LOG4SP_LEVEL_TOTAL; ++i)
    {
        logger.LogAmxTpl(view_as<LogLevel>(i), "test message %d", i);
    }

    logger.Flush();

    logger.Close();

    // log count = 7 [total] - 2 [trace, debug]
    // log post count = 7 [total] - 2 [trace, debug] - 1 [throwException]
    // flush count =  1 [manual]
    // err count = 1 [log] + (7 [logPost] - 2 [trace, debug]) + 1 [flush]
    // destroy count = 1 [CreateLogger]
    AssertGlobal(5, 4, 1, 7, 1);
}

static bool CBSink_OnLog(const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int timePoint, any data, char error[256])
{
    g_iLogCount++;
    AssertStrEq("OnLog name", name, LOGGER_NAME);
    AssertTrue("OnLog lvl", lvl >= LogLevel_Info && lvl <= LogLevel_Off);
    AssertStrMatch("OnLog msg match", msg, "test message [0-9]");
    AssertEq("OnLog data", data, 7);

    if (lvl == LogLevel_Off)
    {
        strcopy(error, sizeof(error), "This error message will be thrown!");
        return true;
    }
    return false;
}

static void CBSink_OnLogPost(const char[] msg, any data)
{
    g_iLogPostCount++;
    char[] pattern = "'[0-9]{4}-[0-9]{2}-[0-9]{2}' '(info|warn|error|fatal|off)' 'test-callback2' 'test message [0-9]'(\n|\r\n)";
    AssertStrMatch("OnLogPost msg match", msg, pattern);
    AssertEq("OnLogPost data", data, 7);
}

static bool CBSink_OnFlush(any data, char error[256])
{
    g_iFlushCount++;
    AssertEq("OnFlush data", data, 7);

    strcopy(error, sizeof(error), "This error message will be thrown!");
    return true;
}

static void CBSink_OnLog2(const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int timePoint, any data)
{
    AssertStrEq("OnLog2 name", name, LOGGER_NAME);
    AssertTrue("OnLog lvl", lvl >= LogLevel_Info && lvl <= LogLevel_Off);
    AssertStrMatch("OnLog2 msg match", msg, "test message [0-9]");
    AssertEq("OnLog2 data", data, 9);
}

static bool CBSink_OnLogPost2(const char[] msg, any data, char error[256])
{
    char[] pattern = "'[0-9]{4}-[0-9]{2}-[0-9]{2}' '(info|warn|error|fatal|off)' 'test-callback2' 'test message [0-9]'(\n|\r\n)";
    AssertStrMatch("OnLogPost2 msg match", msg, pattern);
    AssertEq("OnLogPost2 data", data, 9);

    strcopy(error, sizeof(error), "This error message will be thrown!");
    return true;
}

static void CBSink_OnFlush2(any data)
{
    AssertEq("OnFlush2 data", data, 9);
}

static void CBSink_OnDestory(any data)
{
    g_iDestoryCount++;
    AssertTrue("OnDestory data", data == 7 || data == 9);
}

static void CBLogger_ErrorHandler(const char[] msg)
{
    g_iErrorcount++;
    AssertStrEq("OnError", msg, "This error message will be thrown!");
}


static void InitializeGlobal()
{
    g_iLogCount = 0;
    g_iLogPostCount = 0;
    g_iFlushCount = 0;
    g_iErrorcount = 0;
    g_iDestoryCount = 0;
}

static void AssertGlobal(int logCnt, int logPostCnt, int flushCnt, int errorCnt, int destoryCnt)
{
    DataPack data = new DataPack();
    data.WriteCell(logCnt);
    data.WriteCell(logPostCnt);
    data.WriteCell(flushCnt);
    data.WriteCell(errorCnt);
    data.WriteCell(destoryCnt);
    RequestFrame(Frame_AssertGlobal, data);
}

static void Frame_AssertGlobal(DataPack data)
{
    data.Reset();
    int logCnt = data.ReadCell();
    int logPostCnt = data.ReadCell();
    int flushCnt = data.ReadCell();
    int errorCnt = data.ReadCell();
    int destoryCnt = data.ReadCell();
    delete data;

    AssertEq("Log count", g_iLogCount, logCnt);
    AssertEq("LogPost count", g_iLogPostCount, logPostCnt);
    AssertEq("Flush count", g_iFlushCount, flushCnt);
    AssertEq("Error count", g_iErrorcount, errorCnt);
    AssertEq("Destory count", g_iDestoryCount, destoryCnt);
}

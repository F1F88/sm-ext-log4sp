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
#include "../test_utils"


int __iLog1Count;
int __iLog2Count;
int __iFlush1Count;
int __iFlush2Count;
int __iClose1Count;
int __iClose2Count;
int __iErrorcount;


public void OnPluginStart()
{
    Test();
    RegServerCmd("sm_log4sp_test_callback_logger", Command_Test);
}

Action Command_Test(int args)
{
    Test();
    return Plugin_Handled;
}


void Test()
{
    PrintToServer("--------- Started testing Callback-Logger --------");

    SetTestContext("CallbackSink");

    TestCustomCallbackLogger();

    PrintToServer("----------- Test Callback-Logger ended -----------");
}

void TestCustomCallbackLogger()
{
    __InitializeGlobal();

    CallbackSink sink1 = new CallbackSink(null, CB_OnLog1, null, CB_OnFlush1, null, CB_OnClose1);
    CallbackSink sink2 = new CallbackSink(null, CB_OnLog2, null, CB_OnFlush2, null, CB_OnClose2);
    sink2.SetLevel(LogLevel_Warn);
    Logger logger = new Logger("MyLogger");
    logger.AddSink(sink1);
    logger.AddSink(sink2);
    SinkCleanupAndDelete(sink1);

    logger.SetErrorHandler(null, CB_ErrorHandler);

    for (int i = 0; i < view_as<int>(LogLevel_Total); ++i)
    {
        logger.LogSrc(view_as<LogLevel>(i), "test message.");
    }

    // log1 count   = 5 = 7 [total] - 2 [trace, debug]
    // log2 count   = 4 = 7 [total] - 2 [trace, debug, info]
    __AssertLogCounter(5, 4);

    logger.Flush();
    sink2.Flush();

    // flush1 count = 1 = 1 [logger.Flush]
    // flush2 count = 2 = 1 [logger.Flush] + 1 [sink2.Flush]
    __AssertFlushCounter(1, 2);

    // err count    = 2 = 1 [log LogLevel_Off] + 1 [flush]
    __AssertErrorCounter(2);

    LoggerCleanupAndDelete(logger);
    SinkCleanupAndDelete(sink2);

    // close1 count = 1 = 1 [sink1.Close]
    // close2 count = 1 = 1 [sink2.Close]
    __AssertCloseCounter(1, 1);
}


public bool CB_OnLog1(CallbackSink sink, char[] error, int maxlen, LOG4SP_ES_CONST char[] logTime, LOG4SP_ES_CONST SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg)
{
    __iLog1Count++;
    AssertStrEq("[OnLog1 funcname]", loc.funcname, "TestCustomCallbackLogger");
    AssertStrEq("[OnLog1   name  ]", name, "MyLogger");
    AssertTrue("[OnLog1   lvl   ]", lvl >= LogLevel_Info && lvl <= LogLevel_Off);
    AssertStrEq("[OnLog1   msg   ]", msg, "test message.");

    if (lvl == LogLevel_Off)
    {
        strcopy(error, maxlen, "This error message will be thrown!");
        return false;
    }
    return true;
}

public bool CB_OnLog2(CallbackSink sink, char[] error, int maxlen, const char[] logTime, LOG4SP_ES_CONST SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg)
{
    __iLog2Count++;
    AssertStrEq("[OnLog2 funcname]", loc.funcname, "TestCustomCallbackLogger");
    AssertStrEq("[OnLog2   name  ]", name, "MyLogger");
    AssertTrue("[OnLog2   lvl   ]", lvl >= LogLevel_Info && lvl <= LogLevel_Off);
    AssertStrEq("[OnLog2   msg   ]", msg, "test message.");
    return true;
}

public bool CB_OnFlush1(CallbackSink sink, char[] error, int maxlen)
{
    __iFlush1Count++;
    strcopy(error, maxlen, "This error message will be thrown!");
    return false;
}

public bool CB_OnFlush2(CallbackSink sink, char[] error, int maxlen)
{
    __iFlush2Count++;
    return true;
}

public void CB_OnClose1(any data)
{
    __iClose1Count++;
}

public void CB_OnClose2(any data)
{
    __iClose2Count++;
}

public void CB_ErrorHandler(const char[] origin, LOG4SP_ES_CONST SourceLoc loc, const char[] msg)
{
    __iErrorcount++;
    AssertStrEq("[OnError msg]", msg, "This error message will be thrown!");
}


static void __InitializeGlobal()
{
    __iLog1Count = 0;
    __iLog2Count = 0;
    __iFlush1Count = 0;
    __iFlush2Count = 0;
    __iErrorcount = 0;
    __iClose1Count = 0;
    __iClose2Count = 0;
}

static void __AssertLogCounter(int log1Cnt, int log2Cnt)
{
    AssertEq("[OnLog1 count]", __iLog1Count, log1Cnt);
    AssertEq("[OnLog2 count]", __iLog2Count, log2Cnt);
}

static void __AssertFlushCounter(int flush1Cnt, int flush2Cnt)
{
    AssertEq("[OnFlush1 count]", __iFlush1Count, flush1Cnt);
    AssertEq("[OnFlush2 count]", __iFlush2Count, flush2Cnt);
}

static void __AssertErrorCounter(int errorCnt)
{
    AssertEq("[OnError count]", __iErrorcount, errorCnt);
}

static void __AssertCloseCounter(int close1Cnt, int close2Cnt)
{
    AssertEq("[OnClose1 count]", __iClose1Count, close1Cnt);
    AssertEq("[OnClose2 count]", __iClose2Count, close2Cnt);
}

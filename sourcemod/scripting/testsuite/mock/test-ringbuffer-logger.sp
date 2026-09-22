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


public void OnPluginStart()
{
    Test();
    RegServerCmd("sm_log4sp_test_ringbuffer_logger", Command_Test);
}

Action Command_Test(int args)
{
    Test();
    return Plugin_Handled;
}


void Test()
{
    PrintToServer("------- Started testing Ring-Buffer-Logger -------");

    TestDrain();

    TestEmpty();

    TestEmptySize();

    PrintToServer("---------- Test Ring-Buffer-Logger ended ---------");
}

void TestDrain()
{
    SetTestContext("RingBuffer Drain");

    static const int sinkSize = 3;
    RingBufferSink sink = new RingBufferSink(sinkSize);

    Logger logger = new Logger("MyLogger");
    logger.AddSink(sink);

    // log more than the sink size by one and test that the first message is dropped
    // test 3 times to make sure the ringbuffer is working correctly multiple times
    for (int i = 1; i <= 3; ++i)
    {
        for (int j = i; j <= sinkSize * i; ++j)
        {
            logger.InfoF("%d", j);
        }

        // 缓冲区只能存储预设条数消息
        AssertEq("buffer length", sink.GetSize(), sinkSize);

        // 从最新的消息开始消费
        DataPack data = new DataPack();
        data.WriteCell(sinkSize * i);
        while (sink.DrainLatest(null, CB_OnDrainLatest, data)) {}
        delete data;
    }
    logger.Close();
    sink.Close();
}

void TestEmpty()
{
    SetTestContext("RingBuffer Drain Empty");

    static const int sinkSize = 3;
    RingBufferSink sink = new RingBufferSink(sinkSize);

    Logger logger = new Logger();
    logger.AddSink(sink);

    AssertEq("buffer length", sink.GetSize(), 0);
    sink.DrainLatest(null, CB_OnDrainLatest_Empty);

    logger.Close();
    sink.Close();
}

void TestEmptySize()
{
    SetTestContext("RingBuffer New Empty Size");

    static const int sinkSize = 0;
    RingBufferSink sink = new RingBufferSink(sinkSize);

    Logger logger = new Logger();
    logger.AddSink(sink);

    for (int i = 0; i < sinkSize + 1; ++i)
    {
        logger.InfoF("%d", i);
    }

    AssertEq("buffer length", sink.GetSize(), 0);
    sink.DrainLatest(null, CB_OnDrainLatest_EmptySize);

    logger.Close();
    sink.Close();
}


public void CB_OnDrainLatest(const char[] logTime, SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg, DataPack data)
{
    data.Reset();
    int counter = data.ReadCell();

    AssertStrEq("Drain name", name, "MyLogger");
    AssertEq("Drain lvl", lvl, LOG4SP_LEVEL_INFO);
    AssertEq("Drain msg", StringToInt(msg), counter);

    data.Reset(true);
    data.WriteCell(--counter);
}

public void CB_OnDrainLatest_Empty(const char[] logTime, SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg)
{
    AssertFalse("Drain empty", true); // should not be called since the sink is empty
}

public void CB_OnDrainLatest_EmptySize(const char[] logTime, SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg)
{
    AssertFalse("Drain empty size", true); // should not be called since the sink size is 0
}

#include <log4sp>

#include "test_utils"

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_PATTERN  ".*test-ringbuffer-logger.sp"
#define LOGGER_NAME     "test-ring-buffer"


public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_ringbuffer_logger", Command_Test);
}

Action Command_Test(int args)
{
    PrintToServer("---- START TEST RING BUFFER LOGGER ----");

    TestDrain();

    TestDrainFormatted();

    TestEmpty();

    TestEmptySize();

    PrintToServer("---- STOP TEST RING BUFFER LOGGER ----");
    return Plugin_Handled;
}


void TestDrain()
{
    SetTestContext("Test Drain");

    static const int sinkSize = 3;
    RingBufferSink sink = new RingBufferSink(sinkSize);
    sink.SetPattern("*** %v");

    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // log more than the sink size by one and test that the first message is dropped
    // test 3 times to make sure the ringbuffer is working correctly multiple times
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < sinkSize + 1; ++j)
        {
            logger.InfoAmxTpl("%d", j);
        }

        DataPack data = new DataPack();
        data.WriteCell(1);  // counter
        sink.Drain(RBSink_Drain, data);

        data.Reset();
        AssertEq("counter", data.ReadCell() - 1, sinkSize);
        delete data;
    }
    delete logger;
    delete sink;
}

void TestDrainFormatted()
{
    SetTestContext("Test Drain Formatted");

    static const int sinkSize = 3;
    RingBufferSink sink = new RingBufferSink(sinkSize);
    sink.SetPattern("fmt %v");

    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    // log more than the sink size by one and test that the first message is dropped
    // test 3 times to make sure the ringbuffer is working correctly multiple times
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < sinkSize + 1; ++j)
        {
            logger.InfoAmxTpl("%d", j);
        }

        sink.DrainFormatted(RBSink_DrainFormatted, 2);
    }
    delete logger;
    delete sink;
}

void TestEmpty()
{
    SetTestContext("Test Empty");

    static const int sinkSize = 3;
    RingBufferSink sink = new RingBufferSink(sinkSize);

    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    sink.Drain(RBSink_DrainEmpty);

    delete logger;
    delete sink;
}

void TestEmptySize()
{
    SetTestContext("Test Empty Size");

    static const int sinkSize = 0;
    RingBufferSink sink = new RingBufferSink(sinkSize);

    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    for (int i = 0; i < sinkSize + 1; ++i)
    {
        logger.InfoAmxTpl("%d", i);
    }

    sink.DrainFormatted(RBSink_DrainFormattedEmpty);

    delete logger;
    delete sink;
}


void RBSink_Drain(const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int timePoint, DataPack data)
{
    data.Reset();
    int counter = data.ReadCell();

    AssertStrEq("Drain name", name, LOGGER_NAME);
    AssertEq("Drain lvl", lvl, LogLevel_Info);
    AssertEq("Drain msg", StringToInt(msg), counter);

    data.Reset(true);
    data.WriteCell(++counter);
}

void RBSink_DrainFormatted(const char[] msg, DataPack data)
{
    AssertStrMatch("Drain formatted msg", msg, "fmt [0-9]+\\s");
    AssertEq("Drain formatted data", data, 2);
}


void RBSink_DrainEmpty(const char[] name, LogLevel lvl, const char[] msg)
{
    AssertTrue("Drain empty", false); // should not be called since the sink is empty
}

void RBSink_DrainFormattedEmpty(const char[] msg)
{
    AssertTrue("Drain formatted empty", false); // should not be called since the sink size is 0
}

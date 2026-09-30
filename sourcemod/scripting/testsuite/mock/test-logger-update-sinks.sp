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


const int g_iIters = 500;


public void OnPluginStart()
{
    Test();
    RegServerCmd("sm_log4sp_test_logger_update_sinks", Command_Test);
}

Action Command_Test(int args)
{
    Test();
    return Plugin_Handled;
}


void Test()
{
    PrintToServer("------- Started testing Logger-Update-Sinks ------");

    PrepareTestPath("update-sinks/");

    TestUpdateSinksCase1();
    TestUpdateSinksCase2();
    TestUpdateSinksCase3();
    TestUpdateSinksCase4();
    TestUpdateSinksCase5();
    TestUpdateSinksCase6();
    TestUpdateSinksCase7();

    PrintToServer("--------- Test Logger-Update-Sinks ended ---------");
}

void TestUpdateSinksCase1()
{
    SetTestContext("Update Sinks Case 1");
    char basicFilename1[PLATFORM_MAX_PATH], basicFilename2[PLATFORM_MAX_PATH], basicFilename3[PLATFORM_MAX_PATH];
    BuildTestPath(basicFilename1, sizeof(basicFilename1), "update-sinks/case-1-basic_file.log");
    BuildTestPath(basicFilename2, sizeof(basicFilename2), "update-sinks/case-1-daily_file.log");
    BuildTestPath(basicFilename3, sizeof(basicFilename3), "update-sinks/case-1-rotating_file.log");

    for (int i = 0; i < g_iIters; ++i)
    {
        BasicFileSink sink1 = new BasicFileSink(basicFilename1);
        DailyFileSink sink2 = new DailyFileSink(basicFilename2, .calcFunc = CB_OnDailyFileCalculator);
        RotatingFileSink sink3 = new RotatingFileSink(basicFilename3, 1024 * 1024, 1);
        Logger logger = new Logger();

        logger.AddSink(sink1);
        logger.AddSink(sink2);
        logger.AddSink(sink3);

        logger.InfoF("Test message %d", i);

        logger.DropSink(sink1);
        logger.DropSink(sink2);
        logger.DropSink(sink3);

        sink1.Close();
        sink2.Close();
        sink3.Close();
        logger.Close();
    }

    AssertFileLinesEq("[basic file]", basicFilename1, g_iIters);
    AssertFileLinesEq("[daily file]", basicFilename2, g_iIters);
    AssertFileLinesEq("[rotating file]", basicFilename3, g_iIters);
}

void TestUpdateSinksCase2()
{
    SetTestContext("Update Sinks Case 2");
    char basicFilename1[PLATFORM_MAX_PATH], basicFilename2[PLATFORM_MAX_PATH], basicFilename3[PLATFORM_MAX_PATH];
    BuildTestPath(basicFilename1, sizeof(basicFilename1), "update-sinks/case-2-basic_file.log");
    BuildTestPath(basicFilename2, sizeof(basicFilename2), "update-sinks/case-2-daily_file.log");
    BuildTestPath(basicFilename3, sizeof(basicFilename3), "update-sinks/case-2-rotating_file.log");

    for (int i = 0; i < g_iIters; ++i)
    {
        BasicFileSink sink1 = new BasicFileSink(basicFilename1);
        DailyFileSink sink2 = new DailyFileSink(basicFilename2, .calcFunc = CB_OnDailyFileCalculator);
        RotatingFileSink sink3 = new RotatingFileSink(basicFilename3, 1024 * 1024, 1024);
        Logger logger = new Logger();

        logger.AddSink(sink1);
        logger.AddSink(sink2);
        logger.AddSink(sink3);

        logger.InfoF("Test message %d", i);

        // DropSink

        sink1.Close();
        sink2.Close();
        sink3.Close();
        logger.Close();
    }

    AssertFileLinesEq("[basic file]", basicFilename1, g_iIters);
    AssertFileLinesEq("[daily file]", basicFilename2, g_iIters);
    AssertFileLinesEq("[rotating file]", basicFilename3, g_iIters);
}

void TestUpdateSinksCase3()
{
    SetTestContext("Update Sinks Case 3");
    char basicFilename1[PLATFORM_MAX_PATH], basicFilename2[PLATFORM_MAX_PATH], basicFilename3[PLATFORM_MAX_PATH];
    BuildTestPath(basicFilename1, sizeof(basicFilename1), "update-sinks/case-3-basic_file.log");
    BuildTestPath(basicFilename2, sizeof(basicFilename2), "update-sinks/case-3-daily_file.log");
    BuildTestPath(basicFilename3, sizeof(basicFilename3), "update-sinks/case-3-rotating_file.log");

    for (int i = 0; i < g_iIters; ++i)
    {
        BasicFileSink sink1 = new BasicFileSink(basicFilename1);
        DailyFileSink sink2 = new DailyFileSink(basicFilename2, .calcFunc = CB_OnDailyFileCalculator);
        RotatingFileSink sink3 = new RotatingFileSink(basicFilename3, 1024 * 1024, 1024);
        Logger logger = new Logger();

        logger.AddSink(sink1);
        logger.AddSink(sink2);
        logger.AddSink(sink3);

        // DropSink

        sink1.Close();
        sink2.Close();
        sink3.Close();

        logger.InfoF("Test message %d", i);

        logger.Close();
    }

    AssertFileLinesEq("[basic file]", basicFilename1, g_iIters);
    AssertFileLinesEq("[daily file]", basicFilename2, g_iIters);
    AssertFileLinesEq("[rotating file]", basicFilename3, g_iIters);
}

void TestUpdateSinksCase4()
{
    SetTestContext("Update Sinks Case 4");
    char basicFilename1[PLATFORM_MAX_PATH], basicFilename2[PLATFORM_MAX_PATH], basicFilename3[PLATFORM_MAX_PATH];
    BuildTestPath(basicFilename1, sizeof(basicFilename1), "update-sinks/case-4-basic_file.log");
    BuildTestPath(basicFilename2, sizeof(basicFilename2), "update-sinks/case-4-daily_file.log");
    BuildTestPath(basicFilename3, sizeof(basicFilename3), "update-sinks/case-4-rotating_file.log");

    Logger logger = new Logger();
    for (int i = 0; i < g_iIters; ++i)
    {
        BasicFileSink sink1 = new BasicFileSink(basicFilename1);
        DailyFileSink sink2 = new DailyFileSink(basicFilename2, .calcFunc = CB_OnDailyFileCalculator);
        RotatingFileSink sink3 = new RotatingFileSink(basicFilename3, 1024 * 1024, 1024);
        // new Logger

        logger.AddSink(sink1);
        logger.AddSink(sink2);
        logger.AddSink(sink3);

        logger.InfoF("Test message %d", i);

        logger.DropSink(sink1);
        logger.DropSink(sink2);
        logger.DropSink(sink3);

        sink1.Close();
        sink2.Close();
        sink3.Close();
    }
    logger.Close();

    AssertFileLinesEq("[basic file]", basicFilename1, g_iIters);
    AssertFileLinesEq("[daily file]", basicFilename2, g_iIters);
    AssertFileLinesEq("[rotating file]", basicFilename3, g_iIters);
}

void TestUpdateSinksCase5()
{
    SetTestContext("Update Sinks Case 5");
    char basicFilename1[PLATFORM_MAX_PATH], basicFilename2[PLATFORM_MAX_PATH], basicFilename3[PLATFORM_MAX_PATH];
    BuildTestPath(basicFilename1, sizeof(basicFilename1), "update-sinks/case-5-basic_file.log");
    BuildTestPath(basicFilename2, sizeof(basicFilename2), "update-sinks/case-5-daily_file.log");
    BuildTestPath(basicFilename3, sizeof(basicFilename3), "update-sinks/case-5-rotating_file.log");

    Logger logger = new Logger();
    BasicFileSink sink1 = new BasicFileSink(basicFilename1);
    DailyFileSink sink2 = new DailyFileSink(basicFilename2, .calcFunc = CB_OnDailyFileCalculator);
    RotatingFileSink sink3 = new RotatingFileSink(basicFilename3, 1024 * 1024, 1024);
    for (int i = 0; i < g_iIters; ++i)
    {
        // new Logger
        // new Sink

        logger.AddSink(sink1);
        logger.AddSink(sink2);
        logger.AddSink(sink3);

        logger.InfoF("Test message %d", i);

        logger.DropSink(sink1);
        logger.DropSink(sink2);
        logger.DropSink(sink3);
    }
    sink1.Close();
    sink2.Close();
    sink3.Close();
    logger.Close();

    AssertFileLinesEq("[basic file]", basicFilename1, g_iIters);
    AssertFileLinesEq("[daily file]", basicFilename2, g_iIters);
    AssertFileLinesEq("[rotating file]", basicFilename3, g_iIters);
}

void TestUpdateSinksCase6()
{
    SetTestContext("Update Sinks Case 6");
    char basicFilename1[PLATFORM_MAX_PATH], basicFilename2[PLATFORM_MAX_PATH], basicFilename3[PLATFORM_MAX_PATH];
    BuildTestPath(basicFilename1, sizeof(basicFilename1), "update-sinks/case-6-basic_file.log");
    BuildTestPath(basicFilename2, sizeof(basicFilename2), "update-sinks/case-6-daily_file.log");
    BuildTestPath(basicFilename3, sizeof(basicFilename3), "update-sinks/case-6-rotating_file.log");

    Logger logger = new Logger();
    BasicFileSink sink1 = new BasicFileSink(basicFilename1);
    DailyFileSink sink2 = new DailyFileSink(basicFilename2, .calcFunc = CB_OnDailyFileCalculator);
    for (int i = 0; i < g_iIters; ++i)
    {
        // new Logger
        // new Sink
        RotatingFileSink sink3 = new RotatingFileSink(basicFilename3, 1024 * 1024, 1024);

        logger.AddSink(sink1);
        logger.AddSink(sink2);
        logger.AddSink(sink3);

        logger.InfoF("Test message %d", i);

        logger.DropSink(sink1);
        logger.DropSink(sink2);
        logger.DropSink(sink3);

        sink3.Close();
    }
    sink1.Close();
    sink2.Close();
    logger.Close();

    AssertFileLinesEq("[basic file]", basicFilename1, g_iIters);
    AssertFileLinesEq("[daily file]", basicFilename2, g_iIters);
    AssertFileLinesEq("[rotating file]", basicFilename3, g_iIters);
}

void TestUpdateSinksCase7()
{
    SetTestContext("Update Sinks Case 7");
    char basicFilename1[PLATFORM_MAX_PATH], basicFilename2[PLATFORM_MAX_PATH], basicFilename3[PLATFORM_MAX_PATH];
    BuildTestPath(basicFilename1, sizeof(basicFilename1), "update-sinks/case-7-basic_file.log");
    BuildTestPath(basicFilename2, sizeof(basicFilename2), "update-sinks/case-7-daily_file.log");
    BuildTestPath(basicFilename3, sizeof(basicFilename3), "update-sinks/case-7-rotating_file.log");

    Logger logger = new Logger();
    BasicFileSink sink1 = new BasicFileSink(basicFilename1);
    for (int i = 0; i < g_iIters; ++i)
    {
        // new Logger
        // new Sink
        DailyFileSink sink2 = new DailyFileSink(basicFilename2, .calcFunc = CB_OnDailyFileCalculator);
        RotatingFileSink sink3 = new RotatingFileSink(basicFilename3, 1024 * 1024, 1024);

        logger.AddSink(sink1);
        logger.AddSink(sink2);
        logger.AddSink(sink3);

        logger.InfoF("Test message %d", i);

        logger.DropSink(sink1);
        logger.DropSink(sink2);
        logger.DropSink(sink3);

        sink2.Close();
        sink3.Close();
    }
    sink1.Close();
    logger.Close();

    AssertFileLinesEq("[basic file]", basicFilename1, g_iIters);
    AssertFileLinesEq("[daily file]", basicFilename2, g_iIters);
    AssertFileLinesEq("[rotating file]", basicFilename3, g_iIters);
}


// 仅一个文件不需要计算
public void CB_OnDailyFileCalculator(char[] filename, int maxlen, const char[] timestamp)
{}

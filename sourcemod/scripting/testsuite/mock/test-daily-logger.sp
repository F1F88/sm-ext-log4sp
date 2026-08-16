#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <log4sp>

#include "../test_utils"


public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_daily_logger", Command_Test);
}

Action Command_Test(int args)
{
    PrintToServer("---- START TEST DAILY LOGGER ----");

    PrepareTestPath("daily/");

    TestDefaultCalculator();

    TestFormatCalculator();

    TestRotates();

    TestFileCallback();

    PrintToServer("---- STOP TEST DAILY LOGGER ----");
    return Plugin_Handled;
}

void TestDefaultCalculator()
{
    SetTestContext("Test Daily Default Calculator");

    char path[PLATFORM_MAX_PATH];
    BuildTestPath(path, sizeof(path), "daily/daily_default_calculator.log");

    DailyFileSink sink = new DailyFileSink(path);
    Logger logger = new Logger("test-daily-calc");
    logger.AddSink(sink);

    for (int i = 0; i < 10; ++i)
    {
        logger.InfoEx("Test message %d", i);
    }
    delete logger;
    delete sink;

    FormatTime(path, sizeof(path), "daily/daily_default_calculator_%Y%m%d.log");
    BuildTestPath(path, sizeof(path), path);

    AssertEq("Generated log file, count lines", CountLines(path), 10);
}

void TestFormatCalculator()
{
    SetTestContext("Test Daily Custom Calculator");

    char path[PLATFORM_MAX_PATH];
    BuildTestPath(path, sizeof(path), "daily/daily_custom_calculator_%Y-%m-%d_%H-%M.log");

    DailyFileSink sink = new DailyFileSink(path, 1, 2, true, 0, DailyFileFormatCalculator);
    Logger logger = new Logger("test-daily-custom-calc");
    logger.AddSink(sink);

    for (int i = 0; i < 10; ++i)
    {
        logger.InfoEx("Test message %d", i);
    }
    delete logger;
    delete sink;

    FormatTime(path, sizeof(path), "daily/daily_custom_calculator_%Y-%m-%d_%H-%M.log");
    BuildTestPath(path, sizeof(path), path);

    AssertEq("Generated log file, count lines", CountLines(path), 10);
}

/* Test removal of old files */
void TestRotates()
{
    SetTestContext("Test Daily File Rotate");

    TestRotate(1, 0, 1);
    TestRotate(1, 1, 1);
    TestRotate(1, 3, 1);
    TestRotate(1, 10, 1);

    TestRotate(10, 0, 10);
    TestRotate(10, 1, 1);
    TestRotate(10, 3, 3);
    TestRotate(10, 9, 9);
    TestRotate(10, 10, 10);
    TestRotate(10, 11, 10);
    TestRotate(10, 20, 10);
}

void TestRotate(int daysToRun, int maxDays, int expectedNumFiles)
{
    char path[PLATFORM_MAX_PATH];
    path = PrepareTestPath("daily/rotate/daily_rotate.log");

    DailyFileSink sink = new DailyFileSink(path, 2, 30, true, maxDays);
    for (int i = 0; i < daysToRun; ++i)
    {
        char logTime[21];
        FormatEx(logTime, sizeof(logTime), "%d000000000", GetTime() + 24 * 3600 * i);

        SourceLoc loc = {__BINARY_PATH__, __LINE__, __BINARY_NAME__};

        sink.Log(logTime, loc, "test-daily", LogLevel_Info, "Hello Message");
    }
    delete sink;

    AssertEq("Generated log file, count files", CountFiles(path), expectedNumFiles);
}

void TestFileCallback()
{
    SetTestContext("Test File Callback");

    char path[PLATFORM_MAX_PATH];
    BuildTestPath(path, sizeof(path), "daily/file_callback.log");

    DailyFileSink sink = new DailyFileSink(path, .openPre=OnOpenPre, .closePost=OnClosePost);
    delete sink;
}

void OnOpenPre(const char[] filename)
{
    char path[PLATFORM_MAX_PATH];
    FormatTime(path, sizeof(path), "daily/file_callback_%Y%m%d.log");
    BuildTestPath(path, sizeof(path), path);

    AssertStrEq("OpenPre, file name", filename, path);
    AssertFalse("OpenPre, file exists", FileExists(path));
}

void OnClosePost(const char[] filename)
{
    char path[PLATFORM_MAX_PATH];
    FormatTime(path, sizeof(path), "daily/file_callback_%Y%m%d.log");
    BuildTestPath(path, sizeof(path), path);

    AssertStrEq("ClosePost, file name", filename, path);
    AssertTrue("ClosePost, file exists", FileExists(path));
}



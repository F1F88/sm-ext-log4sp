#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <log4sp>

#include "test_utils"


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

    Logger logger = DailyFileSink.CreateLogger("test-daily-calc", path);
    for (int i = 0; i < 10; ++i)
    {
        logger.InfoAmxTpl("Test message %d", i);
    }
    delete logger;

    FormatTime(path, sizeof(path), "daily/daily_default_calculator_%Y%m%d.log");
    BuildTestPath(path, sizeof(path), path);

    AssertEq("log file msg cnt", CountLines(path), 10);
}

void TestFormatCalculator()
{
    SetTestContext("Test Daily Custom Calculator");

    char path[PLATFORM_MAX_PATH];
    BuildTestPath(path, sizeof(path), "daily/daily_custom_calculator_%Y-%m-%d_%H-%M.log");

    Logger logger = DailyFileSink.CreateLogger("test-daily-custom-calc", path, 1, 2, true, 0, DailyFileFormatCalculator);
    for (int i = 0; i < 10; ++i)
    {
        logger.InfoAmxTpl("Test message %d", i);
    }
    delete logger;

    FormatTime(path, sizeof(path), "daily/daily_custom_calculator_%Y-%m-%d_%H-%M.log");
    BuildTestPath(path, sizeof(path), path);

    AssertEq("log file msg cnt", CountLines(path), 10);
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
        sink.Log("test-daily", LogLevel_Info, "Hello Message",
            __BINARY_PATH__, __LINE__, __BINARY_NAME__, GetTime() + 24 * 3600 * i);
    }
    delete sink;

    AssertEq("Rotate files count", CountFiles(path), expectedNumFiles);
}

void TestFileCallback()
{
    SetTestContext("Test File Callback");

    char path[PLATFORM_MAX_PATH];
    BuildTestPath(path, sizeof(path), "daily/file_callback.log");

    Logger logger = DailyFileSink.CreateLogger("test-daily-file-logger", path, .openPre=OnOpenPre, .closePost=OnClosePost);
    delete logger;
}

void OnOpenPre(const char[] filename)
{
    char path[PLATFORM_MAX_PATH];
    FormatTime(path, sizeof(path), "daily/file_callback_%Y%m%d.log");
    BuildTestPath(path, sizeof(path), path);

    AssertStrEq("File open pre, filename", filename, path);
    AssertFalse("File open pre, file exists", FileExists(path));
}

void OnClosePost(const char[] filename)
{
    char path[PLATFORM_MAX_PATH];
    FormatTime(path, sizeof(path), "daily/file_callback_%Y%m%d.log");
    BuildTestPath(path, sizeof(path), path);

    AssertStrEq("File close post, filename", filename, path);
    AssertTrue("File close post, file exists", FileExists(path));
}



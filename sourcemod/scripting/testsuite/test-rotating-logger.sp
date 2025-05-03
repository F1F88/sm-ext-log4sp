#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <log4sp>

#include "test_utils"


public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_rotate_logger", Command_Test);
}

Action Command_Test(int args)
{
    PrintToServer("---- START TEST ROTATE LOGGER ----");

    TestCalcFileName();

    TestRotateLogger();

    TestAutoRotate();

    TestManualRotate();

    TestFileCallback();

    PrintToServer("---- STOP TEST ROTATE LOGGER ----");
    return Plugin_Handled;
}


// File name calculations
void TestCalcFileName()
{
    SetTestContext("Test Calc File Name");

    char filename[PLATFORM_MAX_PATH];

    RotatingFileSink.CalcFilename(filename, sizeof(filename), "rotated.txt", 3);
    AssertStrEq("Calc file name 1", filename, "rotated.3.txt");

    RotatingFileSink.CalcFilename(filename, sizeof(filename), "rotated", 3);
    AssertStrEq("Calc file name 2", filename, "rotated.3");

    RotatingFileSink.CalcFilename(filename, sizeof(filename), "rotated.txt", 0);
    AssertStrEq("Calc file name 3", filename, "rotated.txt");
}

void TestRotateLogger()
{
    SetTestContext("Test Rotate Logger");

    const int maxSize = 1024 * 10;

    char path[PLATFORM_MAX_PATH];
    path = PrepareTestPath("rotate-file/rotating_log.log");

    Logger logger = RotatingFileSink.CreateLogger("test-rotate-logger", path, maxSize, 0);
    for (int i = 0; i < 10; ++i)
    {
        logger.InfoAmxTpl("Test message %d", i);
    }
    delete logger;

    AssertEq("Simple log, file line cnt", CountLines(path), 10);
}

void TestAutoRotate()
{
    SetTestContext("Test Auto Rotate");

    const int maxSize = 1024 * 10;

    char path[PLATFORM_MAX_PATH];
    path = PrepareTestPath("rotate-file/rotating_auto_rotate.log");

    // make an initial logger to create the first output file
    Logger logger = RotatingFileSink.CreateLogger("test-rotate-logger", path, maxSize, 2, true);
    for (int i = 0; i < 10; ++i)
    {
        logger.InfoAmxTpl("Test message %d", i);
    }
    delete logger;

    logger = RotatingFileSink.CreateLogger("test-rotate-logger", path, maxSize, 2, true);
    for (int i = 0; i < 10; ++i)
    {
        logger.InfoAmxTpl("Test message %d", i);
    }
    logger.Flush();

    AssertEq("Auto rotate pre, file line cnt", CountLines(path), 10);

    for (int i = 0; i < 1000; ++i)
    {
        logger.InfoAmxTpl("Test message %d", i);
    }
    delete logger;

    AssertTrue("Auto rotate post, file size", FileSize(path) <= maxSize);

    BuildTestPath(path, sizeof(path), "rotate-file/rotating_auto_rotate.1.log");
    AssertTrue("Auto rotate post, file 1 size", FileSize(path) <= maxSize);
}

void TestManualRotate()
{
    SetTestContext("Test Manual Rotate");

    const int maxSize = 1024 * 10;

    char path[PLATFORM_MAX_PATH];
    path = PrepareTestPath("rotate-file/rotating_manual_rotate.log");

    RotatingFileSink sink = new RotatingFileSink(path, maxSize, 2);

    Logger logger = new Logger("test-rotate-logger");
    logger.AddSink(sink);
    logger.SetPattern("%v");

    logger.Info("Test message - pre-rotation");
    logger.Flush();

    sink.RotateNow();

    logger.Info("Test message - post-rotation");
    logger.Flush();
    delete sink;
    delete logger;

    AssertTrue("Manual rotate, file size", 0 < FileSize(path) <= maxSize);
    AssertEq("Manual rotate, file line", CountLines(path), 1);

    AssertFileMatch("Manual rotate, file 1 data", path, "Test message - post-rotation\n");
    AssertTrue("Manual rotate, file 1 size", 0 < FileSize(path) <= maxSize);
    AssertEq("Manual rotate, file 1 line", CountLines(path), 1);
}

void TestFileCallback()
{
    SetTestContext("Test File Callback");

    const int maxSize = 1024 * 10;

    char path[PLATFORM_MAX_PATH];
    path = PrepareTestPath("rotate-file/file_callback.log");

    Logger logger = RotatingFileSink.CreateLogger("test-file-logger", path, maxSize, 1, .openPre=OnOpenPre, .closePost=OnClosePost);
    delete logger;
}

void OnOpenPre(const char[] filename)
{
    char path[PLATFORM_MAX_PATH];
    BuildTestPath(path, sizeof(path), "rotate-file/file_callback.log");

    AssertStrEq("File open pre, filename", filename, path);
    AssertFalse("File open pre, file exists", FileExists(path));
}

void OnClosePost(const char[] filename)
{
    char path[PLATFORM_MAX_PATH];
    BuildTestPath(path, sizeof(path), "rotate-file/file_callback.log");

    AssertStrEq("File close post, filename", filename, path);
    AssertTrue("File close post, file exists", FileExists(path));
}


#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <log4sp>

#include "test_utils"


public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_basic_file_logger", Command_Test);
}

Action Command_Test(int args)
{
    PrintToServer("---- START TEST FILE LOGGER ----");

    TestFileLogger();

    TestFlushOn();

    TestTruncate();

    TestFileCallback();

    PrintToServer("---- STOP TEST FILE LOGGER ----");
    return Plugin_Handled;
}


void TestFileLogger()
{
    SetTestContext("Test Simple File Logger");

    char path[PLATFORM_MAX_PATH];
    path = PrepareTestPath("basic-file/simple_file.log");

    Logger logger = BasicFileSink.CreateLogger("test-file-logger", path);
    logger.SetPattern("%v");

    logger.InfoAmxTpl("Test message %d", 1);
    logger.InfoAmxTpl("Test message %d", 2);
    delete logger;

    AssertEq("File line cnt", CountLines(path), 2);
    AssertFileMatch("File contents", path, "Test message 1\\sTest message 2\\s");
}

void TestFlushOn()
{
    SetTestContext("Test FlushOn");

    char path[PLATFORM_MAX_PATH];
    path = PrepareTestPath("basic-file/flush_on.log");

    Logger logger = BasicFileSink.CreateLogger("test-file-logger", path);
    logger.SetPattern("%v");
    logger.SetLevel(LogLevel_Trace);
    logger.FlushOn(LogLevel_Info);

    logger.InfoAmxTpl("Test message %d", 1);

    logger.Trace("Should not be flushed");
    AssertEq("Only info, file line cnt", CountLines(path), 1);
    AssertFileMatch("Only info, file contents", path, "Test message 1\\s");

    logger.InfoAmxTpl("Test message %d", 2);
    delete logger;

    AssertEq("Contain trace, file line cnt", CountLines(path), 3);
    AssertFileMatch("Contain trace, file contents", path, "Test message 1\\sShould not be flushed\\sTest message 2\\s");
}

void TestTruncate()
{
    SetTestContext("Test Truncate");

    char path[PLATFORM_MAX_PATH];
    path = PrepareTestPath("basic-file/truncate.log");

    BasicFileSink sink = new BasicFileSink(path);
    Logger logger = new Logger("test-file-logger");
    logger.AddSink(sink);

    logger.InfoAmxTpl("Test message %f", 3.14);
    logger.InfoAmxTpl("Test message %f", 2.71);
    logger.Flush();

    AssertEq("Truncate pre, file lines", CountLines(path), 2);

    sink.Truncate();
    AssertEq("Truncate post, file lines", CountLines(path), 0);

    logger.InfoAmxTpl("Test message %f", 6.28);
    delete logger;
    delete sink;
    AssertEq("Truncate final, file lines", CountLines(path), 1);
}

void TestFileCallback()
{
    SetTestContext("Test File Callback");

    char path[PLATFORM_MAX_PATH];
    path = PrepareTestPath("basic-file/file_callback.log");

    Logger logger = BasicFileSink.CreateLogger("test-file-logger", path, _, OnOpenPre, OnClosePost);
    logger.SetPattern("%v");
    logger.Info("Some message");
    delete logger;
}

void OnOpenPre(const char[] filename)
{
    char dir[PLATFORM_MAX_PATH];
    BuildTestPath(dir, sizeof(dir), "basic-file");

    char path[PLATFORM_MAX_PATH];
    BuildTestPath(path, sizeof(path), "basic-file/file_callback.log");

    AssertStrEq("File open pre, filename", filename, path);
    AssertFalse("File open pre, dir exists", DirExists(dir));
    AssertFalse("File open pre, file exists", FileExists(path));

    CreateDirectory(dir);
    File file = OpenFile(path, "wt");
    file.WriteString("Hello File Event Callback! ", false);
    file.Flush();
    delete file;

    AssertFileMatch("File contents", path, "Hello File Event Callback!\\s");
}

void OnClosePost(const char[] filename)
{
    char path[PLATFORM_MAX_PATH];
    BuildTestPath(path, sizeof(path), "basic-file/file_callback.log");

    AssertStrEq("File close post, filename", filename, path);
    AssertTrue("File close post, file exists", FileExists(path));

    File file = OpenFile(path, "at");
    file.WriteString("Goodbey File Event Callback!", false);
    delete file;

    AssertFileMatch("File contents", path, "Hello File Event Callback! Some message\\sGoodbey File Event Callback!");
}

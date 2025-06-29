#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <log4sp>

#include "test_utils"


#define LOGGER_NAME     "test-file-logger"


public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_basic_file_logger", Command_Test);
}

Action Command_Test(int args)
{
    PrintToServer("---- START TEST FILE LOGGER ----");

    PrepareTestPath("basic-file/");

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
    BuildTestPath(path, sizeof(path), "basic-file/simple_file.log");

    Logger logger = BasicFileSink.CreateLogger(LOGGER_NAME, path);
    logger.SetPattern("%v");

    logger.InfoAmxTpl("Test message %d", 1);
    logger.InfoAmxTpl("Test message %d", 2);
    delete logger;

    AssertEq("File count lines", CountLines(path), 2);
    AssertFileMatch("File contents match", path, "Test message 1" ... P_EOL ... "Test message 2" ... P_EOL);
}

void TestFlushOn()
{
    SetTestContext("Test Simple File Auto Flush Level");

    char path[PLATFORM_MAX_PATH];
    BuildTestPath(path, sizeof(path), "basic-file/flush_on.log");

    Logger logger = BasicFileSink.CreateLogger(LOGGER_NAME, path);
    logger.SetPattern("%v");
    logger.SetLevel(LogLevel_Trace);
    logger.FlushOn(LogLevel_Info);

    logger.InfoAmxTpl("Test message %d", 1);
    logger.Trace("Should not be flushed");

    AssertEq("Flush by info level, count lines", CountLines(path), 1);
    AssertFileMatch("Flush by info level, contents match", path, "Test message 1" ... P_EOL);

    logger.InfoAmxTpl("Test message %d", 2);
    delete logger;

    AssertEq("Flush by destructor, count lines", CountLines(path), 3);
    AssertFileMatch("Flush by destructor, contents match", path, "Test message 1" ... P_EOL ... "Should not be flushed" ... P_EOL ... "Test message 2" ... P_EOL);
}

void TestTruncate()
{
    SetTestContext("Test Simple File Truncate");

    char path[PLATFORM_MAX_PATH];
    BuildTestPath(path, sizeof(path), "basic-file/truncate.log");

    BasicFileSink sink = new BasicFileSink(path);
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);

    logger.InfoAmxTpl("Test message %f", 3.14);
    logger.InfoAmxTpl("Test message %f", 2.71);
    logger.Flush();
    AssertEq("Truncate pre, count lines", CountLines(path), 2);

    sink.Truncate();
    AssertEq("Truncate post, count lines", CountLines(path), 0);

    logger.InfoAmxTpl("Test message %f", 6.28);
    delete logger;
    delete sink;
    AssertEq("Truncate final, count lines", CountLines(path), 1);
}

void TestFileCallback()
{
    SetTestContext("Test Simple File Open/Close Callback");

    char path[PLATFORM_MAX_PATH];
    BuildTestPath(path, sizeof(path), "basic-file/file_callback.log");

    Logger logger = BasicFileSink.CreateLogger(LOGGER_NAME, path, _, OnOpenPre, OnClosePost);
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

    AssertStrEq("OpenPre, file name", filename, path);
    AssertFalse("OpenPre, file exists", FileExists(path));

    CreateDirectory(dir);
    File file = OpenFile(path, "wt");
    file.WriteString("Hello File Event Callback! ", false);
    file.Flush();
    delete file;

    AssertStrEq("OpenPre, create and write to file", GetFileContents(path), "Hello File Event Callback! ");
}

void OnClosePost(const char[] filename)
{
    char path[PLATFORM_MAX_PATH];
    BuildTestPath(path, sizeof(path), "basic-file/file_callback.log");

    AssertStrEq("ClosePost, file name", filename, path);
    AssertTrue("ClosePost, file exists", FileExists(path));

    File file = OpenFile(path, "at");
    file.WriteString("Goodbey File Event Callback!", false);
    delete file;

    AssertFileMatch("ClosePost, file contents match", path, "Hello File Event Callback! Some message" ... P_EOL ... "Goodbey File Event Callback!");
}

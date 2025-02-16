#include <testing>

#undef REQUIRE_EXTENSIONS
#include <log4sp>

#include "test_utils"

#pragma semicolon 1
#pragma newdecls required


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
    AssertFileRegexEq("File contents", path, "Test message 1\\sTest message 2\\s");
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
    AssertFileRegexEq("Only info, file contents", path, "Test message 1\\s");

    logger.InfoAmxTpl("Test message %d", 2);
    delete logger;

    AssertEq("Contain trace, file line cnt", CountLines(path), 3);
    AssertFileRegexEq("Contain trace, file contents", path, "Test message 1\\sShould not be flushed\\sTest message 2\\s");
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

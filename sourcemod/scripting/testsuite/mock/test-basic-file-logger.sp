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

#include <regex>
#include <sourcemod>
#include <log4sp>

#include "../assert"
#include "../test_utils"


public void OnPluginStart()
{
    Test();
    RegServerCmd("sm_log4sp_test_basic_file_logger", Command_Test);
}

Action Command_Test(int args)
{
    Test();
    return Plugin_Handled;
}


void Test()
{
    PrintToServer("-------- Started testing Basic-File-Logger -------");

    PrepareTestPath("basic-file/");

    TestLog();

    TestFlush();

    TestGetFilename();

    TestTruncate();

    TestFileCallback();

    PrintToServer("---------- Test Basic-File-Logger ended ----------");
}

void TestLog()
{
    SetTestContext("BasicFile Log");

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "basic-file/log.log");

    BasicFileSink sink = new BasicFileSink(filename);
    Logger logger = new Logger();
    logger.AddSink(sink);

    logger.DebugF("Test message %d", 1);
    logger.InfoF("Test message %d", 2);
    logger.InfoF("Test message %d", 3);

    LoggerCleanupAndDelete(logger);
    SinkCleanupAndDelete(sink);

    AssertFileLinesEq("[logs by logger.Info]", filename, 2);
    AssertFileMatch("[logs by logger.Info]", filename, "Test message 2[^\\S ].*Test message 3[^\\S ]");
}

void TestFlush()
{
    SetTestContext("BasicFile Flush");

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "basic-file/flush.log");

    BasicFileSink sink = new BasicFileSink(filename);
    Logger logger = new Logger();
    logger.AddSink(sink);

    logger.SetLevel(LogLevel_Trace);
    logger.SetFlushLevel(LogLevel_Info);

    logger.InfoF("Test message %d", 1);
    logger.Trace("Should not be flushed");

    AssertFileLinesEq("[flush by flush level]", filename, 1);
    AssertFileMatch("[flush by flush level]", filename, "Test message 1[^\\S ]");

    logger.InfoF("Test message %d", 2);

    LoggerCleanupAndDelete(logger);
    SinkCleanupAndDelete(sink);

    AssertFileLinesEq("[flush by sink.Close ]", filename, 3);
    AssertFileMatch("[flush by sink.Close ]", filename, "Test message 1[^\\S ].*Should not be flushed[^\\S ].*Test message 2[^\\S ]");
}

void TestGetFilename()
{
    SetTestContext("BasicFile Filename");

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "basic-file/get_filename.log");

    BasicFileSink sink = new BasicFileSink(filename);

    char buffer[PLATFORM_MAX_PATH];
    sink.GetFilename(buffer, sizeof(buffer));

    SinkCleanupAndDelete(sink);

    AssertStrEq("[sink.GetFilename]", buffer, filename);
}

void TestTruncate()
{
    SetTestContext("BasicFile Truncate");

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "basic-file/truncate.log");

    BasicFileSink sink = new BasicFileSink(filename);
    Logger logger = new Logger();
    logger.AddSink(sink);

    logger.InfoF("Test message %f", 3.14);
    logger.InfoF("Test message %f", 2.71);
    logger.Flush();

    AssertFileLinesEq("[before sink.Truncate]", filename, 2);
    sink.Truncate();
    AssertFileLinesEq("[after  sink.Truncate]", filename, 0);

    logger.InfoF("Test message %f", 6.28);

    LoggerCleanupAndDelete(logger);
    SinkCleanupAndDelete(sink);

    AssertFileLinesEq("[after  sink.Close]", filename, 1);
}

void TestFileCallback()
{
    SetTestContext("BasicFile File Callback");

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "basic-file/file_callback.log");

    BasicFileSink sink = new BasicFileSink(filename, _, null, CB_OnFileOpen, null, CB_OnFileClose);
    Logger logger = new Logger();
    logger.AddSink(sink);

    logger.Info("Some message");

    LoggerCleanupAndDelete(logger);
    SinkCleanupAndDelete(sink);
}


public void CB_OnFileOpen(const char[] filename)
{
    char expectedFilename[PLATFORM_MAX_PATH];
    BuildTestPath(expectedFilename, sizeof(expectedFilename), "basic-file/file_callback.log");

    AssertStrEq("[OnOpen file name]", filename, expectedFilename);
    AssertFalse("[OnOpen file exists]", FileExists(filename));

    File file = OpenFile(filename, "w");
    file.WriteString("Hello File Event Callback! ", false);
    delete file;

    AssertFileLinesEq("[OnOpen file write data]", filename, 1);
    AssertFileMatch("[OnOpen file write data]", filename, "Hello File Event Callback! ");
}

public void CB_OnFileClose(const char[] filename)
{
    char expectedFilename[PLATFORM_MAX_PATH];
    BuildTestPath(expectedFilename, sizeof(expectedFilename), "basic-file/file_callback.log");

    AssertStrEq("[OnClose file name]", filename, expectedFilename);
    AssertTrue("[OnClose file exists]", FileExists(filename));

    File file = OpenFile(filename, "a");
    file.WriteString("Goodbye File Event Callback!", false);
    delete file;

    AssertFileLinesEq("[OnClose file write data]", filename, 2);
    AssertFileMatch("[OnClose file write data]", filename, "Hello File Event Callback! .*Some message[^\\S ]Goodbye File Event Callback!");
}

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
    RegServerCmd("sm_log4sp_test_rotating_logger", Command_Test);
}

Action Command_Test(int args)
{
    Test();
    return Plugin_Handled;
}


void Test()
{
    PrintToServer("------- Started testing Rotate-File-Logger -------");

    PrepareTestPath("rotate-file/");

    TestCalcFileName();

    TestGetFilename();

    TestRotateLogger();

    TestAutoRotate();

    TestManualRotate();

    TestChangeMaxSizeAndMaxFiles();

    TestFileCallback();

    PrintToServer("---------- Test Rotate-File-Logger ended ---------");
}

// File name calculations
void TestCalcFileName()
{
    SetTestContext("RotatingFileSink CalcFilename");

    char filename[PLATFORM_MAX_PATH];

    RotatingFileSink.CalcFilename(filename, sizeof(filename), "rotated.txt", 3);
    AssertStrEq("[index == 3]", filename, "rotated.3.txt");

    RotatingFileSink.CalcFilename(filename, sizeof(filename), "rotated", 3);
    AssertStrEq("[index == 3]", filename, "rotated.3");

    RotatingFileSink.CalcFilename(filename, sizeof(filename), "rotated.txt", 0);
    AssertStrEq("[index == 0]", filename, "rotated.txt");
}

void TestGetFilename()
{
    SetTestContext("RotatingFileSink GetFilename");

    const int MAX_SIZE  = 1024 * 10;
    const int MAX_FILES = 1;

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "rotate-file/get_filename.log");

    RotatingFileSink sink = new RotatingFileSink(filename, MAX_SIZE, MAX_FILES);

    char buffer[PLATFORM_MAX_PATH];
    sink.GetFilename(buffer, sizeof(buffer));

    SinkCloseAndDelete(sink);

    AssertStrEq("[file name]", buffer, filename);
}

void TestRotateLogger()
{
    SetTestContext("RotatingFileSink Logging");

    const int MAX_SIZE  = 1024 * 10;
    const int MAX_FILES = 0;

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "rotate-file/rotating_log.log");

    RotatingFileSink sink = new RotatingFileSink(filename, MAX_SIZE, MAX_FILES);
    Logger logger = new Logger();
    logger.AddSink(sink);
    SinkCloseAndDelete(sink);

    for (int i = 0; i < 10; ++i)
    {
        logger.InfoF("Test message %d", i);
    }
    LoggerCloseAndDelete(logger);

    AssertFileLinesEq("[Ten msgs]", filename, 10);
}

void TestAutoRotate()
{
    SetTestContext("RotatingFileSink Rotate-Auto");

    const int MAX_SIZE  = 1024 * 10;
    const int MAX_FILES = 2;

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "rotate-file/rotating_auto_rotate.log");

    // make an initial logger to create the first output file
    RotatingFileSink sink = new RotatingFileSink(filename, MAX_SIZE, MAX_FILES, true);
    Logger logger = new Logger();
    logger.AddSink(sink);
    SinkCloseAndDelete(sink);

    for (int i = 0; i < 10; ++i)
    {
        logger.InfoF("Test message %d", i);
    }
    LoggerCloseAndDelete(logger);

    sink = new RotatingFileSink(filename, MAX_SIZE, MAX_FILES, true);
    logger = new Logger();
    logger.AddSink(sink);
    SinkCloseAndDelete(sink);

    for (int i = 0; i < 10; ++i)
    {
        logger.InfoF("Test message %d", i);
    }
    logger.Flush();

    AssertFileLinesEq("[Before rotate]", filename, 10);

    for (int i = 0; i < 1000; ++i)
    {
        logger.InfoF("Test message %d", i);
    }
    LoggerCloseAndDelete(logger);

    AssertLe("[After rotate file _ size]", FileSize(filename), MAX_SIZE);

    BuildTestPath(filename, sizeof(filename), "rotate-file/rotating_auto_rotate.1.log");
    AssertLe("[After rotate file 1 size]", FileSize(filename), MAX_SIZE);
}

void TestManualRotate()
{
    SetTestContext("RotatingFileSink Rotate-Manual");

    const int MAX_SIZE  = 1024 * 10;
    const int MAX_FILES = 2;

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "rotate-file/rotating_manual_rotate.log");

    RotatingFileSink sink = new RotatingFileSink(filename, MAX_SIZE, MAX_FILES);
    Logger logger = new Logger();
    logger.AddSink(sink);

    logger.Info("Test message - pre-rotation");
    logger.Flush();

    AssertTrue("[RotateNow]", sink.RotateNow());
    SinkCloseAndDelete(sink);

    logger.Info("Test message - post-rotation");
    LoggerCloseAndDelete(logger);

    AssertGt("[File Size]", FileSize(filename), 0);
    AssertLe("[File Size]", FileSize(filename), MAX_SIZE);
    AssertFileLinesEq("[File Line]", filename, 1);
    AssertFileMatch("[File Data]", filename, "Test message - post-rotation[^\\S ]");
}

void TestChangeMaxSizeAndMaxFiles()
{
    SetTestContext("RotatingFileSink Change-MaxSize");

    int maxSize = 5 * 1024;
    int maxFiles = 2;

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "rotate-file/rotating_change_max_size.log");

    RotatingFileSink sink = new RotatingFileSink(filename, maxSize, maxFiles);
    Logger logger = new Logger();
    logger.AddSink(sink);

    AssertEq("[Max Size by constructor]", sink.GetMaxSize(), maxSize);
    AssertEq("[Max Files by constructor]", sink.GetMaxFiles(), maxFiles);

    maxSize = 7 * 1024;
    maxFiles = 3;

    sink.SetMaxSize(maxSize);
    sink.SetMaxFiles(maxFiles);

    AssertEq("[Max Size by setter]", sink.GetMaxSize(), maxSize);
    AssertEq("[Max Files by setter]", sink.GetMaxFiles(), maxFiles);

    SinkCloseAndDelete(sink);

#if !defined LOG4SP_HEADER_ONLY
    // 54: [2001-02-03 12:34:56.789] [test-rotate-logger] [info] ...
    char[] msg = "0123456789012345678901234567890123456789012345";
    int msgSize = 54 + strlen(msg);
#else
    // 50: [2001-02-03 12:34:56] [test-rotate-logger] [info] ...
    char[] msg = "01234567890123456789012345678901234567890123456789";
    int msgSize = 50 + strlen(msg);
#endif
    // size + ' ' + 50
    AssertLt("[Msg Size]", strlen(msg), maxSize);
    int numbers = maxFiles * maxSize / msgSize;
    for (int i = 0; i < numbers; ++i) {
        logger.Info(msg);
    }
    LoggerCloseAndDelete(logger); // force flush and close the file

    // validate that the files were rotated correctly with the new max size and max files
    char buffer[PLATFORM_MAX_PATH];
    for (int i = 0; i <= maxFiles; ++i) {
        RotatingFileSink.CalcFilename(buffer, sizeof(buffer), "rotate-file/rotating_change_max_size.log", i);
        BuildTestPath(buffer, sizeof(buffer), buffer);

        int fileSize = FileSize(buffer);

        AssertLt("[All File Size]", fileSize, maxSize);
        if (i > 0)
            AssertGt("[Num File Size]", fileSize, maxSize - msgSize - 2);
    }
}

void TestFileCallback()
{
    SetTestContext("RotatingFileSink File Callback");

    const int MAX_SIZE  = 1024 * 10;
    const int MAX_FILES = 1;

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "rotate-file/file_callback.log");

    RotatingFileSink sink = new RotatingFileSink(filename, MAX_SIZE, MAX_FILES, _, null, CB_OnFileOpen, null, CB_OnFileClose);
    SourceLoc loc;
    sink.Log(NULL_STRING, loc, NULL_STRING, LogLevel_Info, "Some message");
    SinkCloseAndDelete(sink);
}


public void CB_OnFileOpen(const char[] filename)
{
    char expectedFilename[PLATFORM_MAX_PATH];
    BuildTestPath(expectedFilename, sizeof(expectedFilename), "rotate-file/file_callback.log");

    AssertStrEq("[OnOpen file name]", filename, expectedFilename);
    AssertFalse("[OnOpen file exists]", FileExists(expectedFilename));

    File file = OpenFile(filename, "w");
    file.WriteString("Hello File Event Callback! ", false);
    delete file;

    AssertFileLinesEq("[OnOpen file write data]", filename, 1);
    AssertFileMatch("[OnOpen file write data]", filename, "Hello File Event Callback! ");
}

public void CB_OnFileClose(const char[] filename)
{
    char expectedFilename[PLATFORM_MAX_PATH];
    BuildTestPath(expectedFilename, sizeof(expectedFilename), "rotate-file/file_callback.log");

    AssertStrEq("[OnClose file name]", filename, expectedFilename);
    AssertTrue("[OnClose file exists]", FileExists(expectedFilename));

    File file = OpenFile(filename, "a");
    file.WriteString("Goodbye File Event Callback!", false);
    delete file;

    AssertFileLinesEq("[OnClose file write data]", filename, 2);
    AssertFileMatch("[OnClose file write data]", filename, "Hello File Event Callback! .*Some message[^\\S ]Goodbye File Event Callback!");
}


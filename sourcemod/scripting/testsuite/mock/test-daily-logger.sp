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
    RegServerCmd("sm_log4sp_test_daily_logger", Command_Test);
}

Action Command_Test(int args)
{
    Test();
    return Plugin_Handled;
}


void Test()
{
    PrintToServer("-------- Started testing Daily-File-Logger -------");

    PrepareTestPath("daily/");

    TestDefaultCalculator();

    TestFormatCalculator();

    TestGetFilename();

    TestRotates();

    TestFileCallback();

    PrintToServer("---------- Test Daily-File-Logger ended ----------");
}

void TestDefaultCalculator()
{
    SetTestContext("DailyFile Calculator Default");

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "daily/daily_default_calculator.log");

    DailyFileSink sink = new DailyFileSink(filename);
    Logger logger = new Logger("test-daily-default-calc");
    logger.AddSink(sink);
    for (int i = 0; i < 10; ++i)
    {
        logger.InfoF("Test message %d", i);
    }

    sink.GetFilename(filename, sizeof(filename));

    sink.Close();
    logger.Close();

    char buffer[PLATFORM_MAX_PATH];
    FormatTime(buffer, sizeof(buffer), "daily/daily_default_calculator_%Y%m%d.log");
    BuildTestPath(buffer, sizeof(buffer), buffer);

    AssertStrEq("[file name]", filename, buffer);
    AssertFileLinesEq("[file line]", filename, 10);
}

void TestFormatCalculator()
{
    SetTestContext("DailyFile Calculator Format");

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "daily/daily_format_calculator_%Y-%m-%d_%H-%M.log");

    DailyFileSink sink = new DailyFileSink(filename, 1, 2, true, 0, null, DailyFileFormatCalculator);
    Logger logger = new Logger("test-daily-format-calc");
    logger.AddSink(sink);
    for (int i = 0; i < 10; ++i)
    {
        logger.InfoF("Test message %d", i);
    }

    sink.GetFilename(filename, sizeof(filename));

    sink.Close();
    logger.Close();

    char buffer[PLATFORM_MAX_PATH];
    FormatTime(buffer, sizeof(buffer), "daily/daily_format_calculator_%Y-%m-%d_%H-%M.log");
    BuildTestPath(buffer, sizeof(buffer), buffer);

    AssertStrEq("[file name]", filename, buffer);
    AssertFileLinesEq("[file line]", filename, 10);
}

void TestGetFilename()
{
    SetTestContext("DailyFile GetFilename");

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "daily/get_filename.log");

    DailyFileSink sink = new DailyFileSink(filename);

    char buffer[PLATFORM_MAX_PATH];
    sink.GetFilename(buffer, sizeof(buffer));

    sink.Close();

    FormatTime(filename, sizeof(filename), "daily/get_filename_%Y%m%d.log");
    BuildTestPath(filename, sizeof(filename), filename);

    AssertStrEq("[file name]", buffer, filename);
}

/* Test removal of old files */
void TestRotates()
{
    SetTestContext("DailyFile Rotate");

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
    char filename[PLATFORM_MAX_PATH];
    FormatEx(filename, sizeof(filename), "daily/rotate_%d_%d_%d", daysToRun, maxDays, expectedNumFiles);
    if (DirExists(filename))
        AssertTrue("Directory already exists", false);

    Format(filename, sizeof(filename), "%s/daily_rotate.log", filename);
    BuildTestPath(filename, sizeof(filename), filename);

    DailyFileSink sink = new DailyFileSink(filename, 2, 30, true, maxDays);

    int time = GetTime(); // Current day

    for (int i = 0; i < daysToRun; ++i)
    {
        char logTime[21];
        FormatEx(logTime, sizeof(logTime), "%d000000000", time); // To nanoseconds

        SourceLoc loc = {__BINARY_PATH__, __LINE__, __BINARY_NAME__};

        sink.Log(logTime, loc, "test-daily", LogLevel_Info, "Hello Message");

        time += 86400; // Next day
    }
    sink.Close();

    char name[64];
    FormatEx(name, sizeof(name), "[daysToRun=%2d, maxDays=%2d]", daysToRun, maxDays);
    AssertFilesEq(name, filename, expectedNumFiles);
}

void TestFileCallback()
{
    SetTestContext("DailyFile File Callback");

    char filename[PLATFORM_MAX_PATH];
    BuildTestPath(filename, sizeof(filename), "daily/file_callback.log");
    DailyFileSink sink = new DailyFileSink(filename, _, _, _, _, _, _, null, CB_OnFileOpen, null, CB_OnFileClose);
    SourceLoc loc;
    sink.Log(NULL_STRING, loc, "daily-sink", LogLevel_Info, "Some message");
    sink.Close();
}


public void CB_OnFileOpen(const char[] filename)
{
    char expectedFilename[PLATFORM_MAX_PATH];
    FormatTime(expectedFilename, sizeof(expectedFilename), "daily/file_callback_%Y%m%d.log");
    BuildTestPath(expectedFilename, sizeof(expectedFilename), expectedFilename);

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
    FormatTime(expectedFilename, sizeof(expectedFilename), "daily/file_callback_%Y%m%d.log");
    BuildTestPath(expectedFilename, sizeof(expectedFilename), expectedFilename);

    AssertStrEq("[OnClose file name]", filename, expectedFilename);
    AssertTrue("[OnClose file exists]", FileExists(filename));

    File file = OpenFile(filename, "a");
    file.WriteString("Goodbye File Event Callback!", false);
    delete file;

    AssertFileLinesEq("[OnClose file write data]", filename, 2);
    AssertFileMatch("[OnClose file write data]", filename, "Hello File Event Callback! .*Some message[^\\S ]Goodbye File Event Callback!");
}



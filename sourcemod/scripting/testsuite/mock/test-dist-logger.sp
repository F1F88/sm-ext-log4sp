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
    RegServerCmd("sm_log4sp_test_dist_logger", Command_Test);
}

Action Command_Test(int args)
{
    Test();
    return Plugin_Handled;
}


void Test()
{
    PrintToServer("-------- Started testing Dist-Logger -------");

    PrepareTestPath("dist/");

    TestLog();

    TestFlush();

    PrintToServer("---------- Test Dist-Logger ended ----------");
}

void TestLog()
{
    SetTestContext("Dist Log");

    const int MAX_SIZE  = 1024 * 10;
    const int MAX_FILES = 1;

    char filename[6][PLATFORM_MAX_PATH];
    BuildTestPath(filename[0], sizeof(filename[]), "dist/log/basic-file-1.log");
    BuildTestPath(filename[1], sizeof(filename[]), "dist/log/basic-file-2.log");
    BuildTestPath(filename[2], sizeof(filename[]), "dist/log/daily-file-1.log");
    BuildTestPath(filename[3], sizeof(filename[]), "dist/log/daily-file-2.log");
    BuildTestPath(filename[4], sizeof(filename[]), "dist/log/rotate-file-1.log");
    BuildTestPath(filename[5], sizeof(filename[]), "dist/log/rotate-file-2.log");

    Sink sinks[6];
    sinks[0] = new BasicFileSink(filename[0]);
    sinks[1] = new BasicFileSink(filename[1]);
    sinks[2] = new DailyFileSink(filename[2]);
    sinks[3] = new DailyFileSink(filename[3]);
    sinks[4] = new RotatingFileSink(filename[4], MAX_SIZE, MAX_FILES);
    sinks[5] = new RotatingFileSink(filename[5], MAX_SIZE, MAX_FILES);

    DistSink sink = new DistSink();
    sink.AddSink(sinks[0]);
    sink.AddSink(sinks[1]);
    sink.AddSink(sinks[2]);
    sink.AddSink(sinks[3]);
    sink.AddSink(sinks[4]);
    sink.AddSink(sinks[5]);

    Logger logger = new Logger();
    logger.AddSink(sink);

    logger.DebugF("Test message %d", 1);
    logger.InfoF("Test message %d", 2);
    logger.InfoF("Test message %d", 3);

    LoggerCloseAndDelete(logger);
    SinkCloseAndDelete(sinks[0]);
    SinkCloseAndDelete(sinks[1]);
    SinkCloseAndDelete(sinks[2]);
    SinkCloseAndDelete(sinks[3]);
    SinkCloseAndDelete(sinks[4]);
    SinkCloseAndDelete(sinks[5]);
    SinkCloseAndDelete(sink);

    char dailyFilename[2][PLATFORM_MAX_PATH];
    FormatTime(dailyFilename[0], sizeof(dailyFilename[]), "dist/log/daily-file-1_%Y%m%d.log");
    FormatTime(dailyFilename[1], sizeof(dailyFilename[]), "dist/log/daily-file-2_%Y%m%d.log");
    BuildTestPath(dailyFilename[0], sizeof(dailyFilename[]), dailyFilename[0]);
    BuildTestPath(dailyFilename[1], sizeof(dailyFilename[]), dailyFilename[1]);

    AssertFileLinesEq("[logs by logger.Info]", filename[0], 2);
    AssertFileLinesEq("[logs by logger.Info]", filename[1], 2);
    AssertFileLinesEq("[logs by logger.Info]", dailyFilename[0], 2);
    AssertFileLinesEq("[logs by logger.Info]", dailyFilename[1], 2);
    AssertFileLinesEq("[logs by logger.Info]", filename[4], 2);
    AssertFileLinesEq("[logs by logger.Info]", filename[5], 2);
    AssertFileMatch("[logs by logger.Info]", filename[0], "Test message 2[^\\S ].*Test message 3[^\\S ]");
    AssertFileMatch("[logs by logger.Info]", filename[1], "Test message 2[^\\S ].*Test message 3[^\\S ]");
    AssertFileMatch("[logs by logger.Info]", dailyFilename[0], "Test message 2[^\\S ].*Test message 3[^\\S ]");
    AssertFileMatch("[logs by logger.Info]", dailyFilename[1], "Test message 2[^\\S ].*Test message 3[^\\S ]");
    AssertFileMatch("[logs by logger.Info]", filename[4], "Test message 2[^\\S ].*Test message 3[^\\S ]");
    AssertFileMatch("[logs by logger.Info]", filename[5], "Test message 2[^\\S ].*Test message 3[^\\S ]");
}

void TestFlush()
{
    SetTestContext("Dist Flush");

    const int MAX_SIZE  = 1024 * 10;
    const int MAX_FILES = 1;

    char filename[6][PLATFORM_MAX_PATH];
    BuildTestPath(filename[0], sizeof(filename[]), "dist/flush/basic-file-1.log");
    BuildTestPath(filename[1], sizeof(filename[]), "dist/flush/basic-file-2.log");
    BuildTestPath(filename[2], sizeof(filename[]), "dist/flush/daily-file-1.log");
    BuildTestPath(filename[3], sizeof(filename[]), "dist/flush/daily-file-2.log");
    BuildTestPath(filename[4], sizeof(filename[]), "dist/flush/rotate-file-1.log");
    BuildTestPath(filename[5], sizeof(filename[]), "dist/flush/rotate-file-2.log");

    DistSink sink = new DistSink();
    sink.AddSinkAndClose(new BasicFileSink(filename[0]));
    sink.AddSinkAndClose(new BasicFileSink(filename[1]));
    sink.AddSinkAndClose(new DailyFileSink(filename[2]));
    sink.AddSinkAndClose(new DailyFileSink(filename[3]));
    sink.AddSinkAndClose(new RotatingFileSink(filename[4], MAX_SIZE, MAX_FILES));
    sink.AddSinkAndClose(new RotatingFileSink(filename[5], MAX_SIZE, MAX_FILES));

    Logger logger = new Logger();
    logger.AddSinkAndClose(sink);

    logger.SetLevel(LogLevel_Trace);
    logger.SetFlushLevel(LogLevel_Info);

    logger.InfoF("Test message %d", 1);
    logger.Trace("Should not be flushed");

    char dailyFilename[2][PLATFORM_MAX_PATH];
    FormatTime(dailyFilename[0], sizeof(dailyFilename[]), "dist/flush/daily-file-1_%Y%m%d.log");
    FormatTime(dailyFilename[1], sizeof(dailyFilename[]), "dist/flush/daily-file-2_%Y%m%d.log");
    BuildTestPath(dailyFilename[0], sizeof(dailyFilename[]), dailyFilename[0]);
    BuildTestPath(dailyFilename[1], sizeof(dailyFilename[]), dailyFilename[1]);

    AssertFileLinesEq("[flush by flush level]", filename[0], 1);
    AssertFileLinesEq("[flush by flush level]", filename[1], 1);
    AssertFileLinesEq("[flush by flush level]", dailyFilename[0], 1);
    AssertFileLinesEq("[flush by flush level]", dailyFilename[1], 1);
    AssertFileLinesEq("[flush by flush level]", filename[4], 1);
    AssertFileLinesEq("[flush by flush level]", filename[5], 1);
    AssertFileMatch("[flush by flush level]", filename[0], "Test message 1[^\\S ]");
    AssertFileMatch("[flush by flush level]", filename[1], "Test message 1[^\\S ]");
    AssertFileMatch("[flush by flush level]", dailyFilename[0], "Test message 1[^\\S ]");
    AssertFileMatch("[flush by flush level]", dailyFilename[1], "Test message 1[^\\S ]");
    AssertFileMatch("[flush by flush level]", filename[4], "Test message 1[^\\S ]");
    AssertFileMatch("[flush by flush level]", filename[5], "Test message 1[^\\S ]");

    logger.InfoF("Test message %d", 2);

    LoggerCloseAndDelete(logger);

    AssertFileLinesEq("[flush by sink.Close ]", filename[0], 3);
    AssertFileLinesEq("[flush by sink.Close ]", filename[1], 3);
    AssertFileLinesEq("[flush by sink.Close ]", dailyFilename[0], 3);
    AssertFileLinesEq("[flush by sink.Close ]", dailyFilename[1], 3);
    AssertFileLinesEq("[flush by sink.Close ]", filename[4], 3);
    AssertFileLinesEq("[flush by sink.Close ]", filename[5], 3);
    AssertFileMatch("[flush by sink.Close ]", filename[0], "Test message 1[^\\S ].*Should not be flushed[^\\S ].*Test message 2[^\\S ]");
    AssertFileMatch("[flush by sink.Close ]", filename[1], "Test message 1[^\\S ].*Should not be flushed[^\\S ].*Test message 2[^\\S ]");
    AssertFileMatch("[flush by sink.Close ]", dailyFilename[0], "Test message 1[^\\S ].*Should not be flushed[^\\S ].*Test message 2[^\\S ]");
    AssertFileMatch("[flush by sink.Close ]", dailyFilename[1], "Test message 1[^\\S ].*Should not be flushed[^\\S ].*Test message 2[^\\S ]");
    AssertFileMatch("[flush by sink.Close ]", filename[4], "Test message 1[^\\S ].*Should not be flushed[^\\S ].*Test message 2[^\\S ]");
    AssertFileMatch("[flush by sink.Close ]", filename[5], "Test message 1[^\\S ].*Should not be flushed[^\\S ].*Test message 2[^\\S ]");
}

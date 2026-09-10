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
#include "../test_sink"
#include "../test_utils"


public void OnPluginStart()
{
    Test();
    RegServerCmd("sm_log4sp_test_logger_err_handler", Command_Test);
}

Action Command_Test(int args)
{
    Test();
    return Plugin_Handled;
}


void Test()
{
    PrintToServer("------ Started testing Logger-Error-Handler ------");

    PrepareTestPath("test-err-handler/");

    TestDefaultErrorHandler();

    TestCustomErrorHandler();

    PrintToServer("--------- Test Logger-Error-Handler ended --------");
}

void TestDefaultErrorHandler()
{
    SetTestContext("ErrorHandler Default");

    TestSink sink = new TestSink();
    Logger logger = new Logger();
    logger.AddSink(sink);

    logger.InfoF("Test message %d", 1);

    __TryBackupSMErrorFile();
    sink.SetLogError("Encountered an error!");

    logger.InfoF("Test message %d", 2);

    sink.ClearLogError();
    char errorMessages[2048];
    __ReadSMErrorFile(errorMessages, sizeof(errorMessages));
    __TryRestoreSMErrorFile();

    logger.InfoF("Test message %d", 3);
    LoggerCleanupAndDelete(logger);

    int logCount = sink.GetLogCount();
    char messages[2][sizeof(LogEvent::msg)];
    strcopy(messages[0], sizeof(LogEvent::msg), sink.DrainOldest().msg);
    strcopy(messages[1], sizeof(LogEvent::msg), sink.DrainOldest().msg);
    SinkCleanupAndDelete(sink);

    char expectedMessages[][] = {"Test message 1", "Test message 3"};
    char expectedErrorMessagesPattern[] = "L .*: \\[LOG4SP|.*test-logger-err-handler.smx\\] \\[.*test-logger-err-handler.sp::[0-9]+\\] \\[.*test-logger-err-handler\\.smx\\] Encountered an error![^\\S ]";

    AssertEq("[Skip log error]", logCount, 2);
    AssertStrArrayEq("[Skip log error]", messages, expectedMessages, sizeof(expectedMessages));
    AssertStrMatch("[SM error file]", errorMessages, expectedErrorMessagesPattern);
}

int __iCustomErrCnt = 0;
void TestCustomErrorHandler()
{
    SetTestContext("ErrorHandler Custom");

    __iCustomErrCnt = 0;

    TestSink sink = new TestSink();
    Logger logger = new Logger("MyLogger");
    logger.AddSink(sink);

    logger.SetFlushLevel(LogLevel_Info);
    logger.SetErrorHandler(null, CB_ErrorHandler);

    logger.InfoF("Test message %d", 1);
    logger.InfoF("Test message %d", 2);

    sink.SetLogError("Encountered an error!");
    logger.InfoF("Test message %d", 3);
    logger.InfoF("Test message %d", 4);
    logger.InfoF("Test message %d", 5);
    sink.ClearLogError();

    logger.InfoF("Test message %d", 6);
    logger.InfoF("Test message %d", 7);

    int logCount = sink.GetLogCount();
    char messages[4][sizeof(LogEvent::msg)];
    strcopy(messages[3], sizeof(LogEvent::msg), sink.DrainLatest().msg);
    strcopy(messages[2], sizeof(LogEvent::msg), sink.DrainLatest().msg);
    strcopy(messages[1], sizeof(LogEvent::msg), sink.DrainLatest().msg);
    strcopy(messages[0], sizeof(LogEvent::msg), sink.DrainLatest().msg);

    LoggerCleanupAndDelete(logger);
    SinkCleanupAndDelete(sink);

    char expectedMessages[][] = {"Test message 1", "Test message 2", "Test message 6", "Test message 7"};

    AssertEq("[custom callback called]", __iCustomErrCnt, 3);
    AssertEq("[Skip log error]", logCount, 4);
    AssertStrArrayEq("[Skip log error]", messages, expectedMessages, sizeof(expectedMessages));
}


public void CB_ErrorHandler(const char[] origin, SourceLoc loc, const char[] msg)
{
    __iCustomErrCnt++;

    AssertStrEq("[custom callback origin]", origin, "MyLogger");
    AssertStrEndsWith("[custom callback file  ]", loc.filename, "test-logger-err-handler.sp");
    AssertStrEq("[custom callback func  ]", loc.funcname, "TestCustomErrorHandler");
    AssertStrEq("[custom callback msg   ]", msg, "Encountered an error!");
}

static void __TryBackupSMErrorFile()
{
    char errorFile[PLATFORM_MAX_PATH];
    FormatTime(errorFile, sizeof(errorFile), "addons/sourcemod/logs/errors_%Y%m%d.log");
    if (!FileExists(errorFile))
        return;

    char backFile[PLATFORM_MAX_PATH];
    BuildTestPath(backFile, sizeof(backFile), "sm-error-bk.log");

    if (!RenameFile(backFile, errorFile))
        ThrowError("Failed to back up file \"%s\" to file \"%s\".", errorFile, backFile);
    else
        PrintToServer("Back up file \"%s\" to file \"%s\".", errorFile, backFile);
}

static int __ReadSMErrorFile(char[] buffer, int maxlen)
{
    char errorFile[PLATFORM_MAX_PATH];
    FormatTime(errorFile, sizeof(errorFile), "addons/sourcemod/logs/errors_%Y%m%d.log");

    File file = OpenFile(errorFile, "rb");
    int bytes = file.ReadString(buffer, maxlen);
    delete file;
    return bytes;
}

static void __TryRestoreSMErrorFile()
{
    char backFile[PLATFORM_MAX_PATH];
    BuildTestPath(backFile, sizeof(backFile), "sm-error-bk.log");

    // 1 - 没有备份: 删除测试痕迹
    // 2 - 已有备份: 删除测试痕迹并恢复
    char errorFile[PLATFORM_MAX_PATH];
    FormatTime(errorFile, sizeof(errorFile), "addons/sourcemod/logs/errors_%Y%m%d.log");
    if (FileExists(errorFile) && !DeleteFile(errorFile))
        ThrowError("Failed to delete file \"%s\".", errorFile);

    if (!FileExists(backFile))
        return;

    if (!RenameFile(errorFile, backFile))
        ThrowError("Failed to restore file \"%s\" to file \"%s\".", backFile, errorFile);
    else
        PrintToServer("Restore file \"%s\" to file \"%s\".", errorFile, backFile);
}

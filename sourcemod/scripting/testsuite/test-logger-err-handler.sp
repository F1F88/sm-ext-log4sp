#include <regex>
#include <log4sp>

#include "test_utils"

#pragma semicolon 1
#pragma newdecls required


#define LOGGER_NAME                 "test-err-handler"
#define PLUGIN_PATTERN              ".*test-logger-err-handler.sp"
#define ERR_MSG_PATTERN             "String formatted incorrectly - parameter [0-9] \\(total [0-9]\\)"
#define SM_ERR_FILE_PATTERN         "L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[.*\\] \\["... PLUGIN_PATTERN ... "::[0-9]+\\] \\["... LOGGER_NAME ... "\\] " ... ERR_MSG_PATTERN
#define DEFAULT_LOG_FILE_PATTERN    "Test message 1\nTest message 3\n"
#define CUSTOM_LOG_FILE_PATTERN     "Test message 1\nTest message 2\nTest message 6\nTest message 7\n"


public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_logger_err_handler", Command_Test);
}

Action Command_Test(int args)
{
    PrintToServer("---- START TEST LOGGER ERR HANDLER ----");

    TestDefaultErrorHandler();

    TestCustomErrorHandler();

    PrintToServer("---- STOP TEST LOGGER ERR HANDLER ----");
    return Plugin_Handled;
}


void TestDefaultErrorHandler()
{
    SetTestContext("Test Default Error Handler");

    // SM 错误日志文件路径
    char errFilePath[PLATFORM_MAX_PATH];
    FormatTime(errFilePath, sizeof(errFilePath), "addons/sourcemod/logs/errors_%Y%m%d.log");

    // 读取当前行数, 将指针置于文件末尾
    LogError("======= Test Default Error Handler =======");
    int errStartlines = 0;
    File errFile = OpenFile(errFilePath, "r");
    if (errFile)
    {
        errStartlines = CountLines(errFilePath);
        errFile.Seek(0, SEEK_END);
    }

    char path[PLATFORM_MAX_PATH];
    path = PrepareTestPath("err-handler/default_handler.log");

    Logger logger = BasicFileSink.CreateLogger(LOGGER_NAME, path);
    logger.SetPattern("%v");

    logger.InfoEx("Test message %d", 1);
    logger.InfoEx("Test message %d %d", 2);
    logger.InfoEx("Test message %d", 3);
    delete logger;

    AssertEq("SM file line cnt", CountLines(errFilePath), errStartlines + 1);
    AssertFileRegexEq2("SM file contents", errFile, SM_ERR_FILE_PATTERN);
    delete errFile;

    AssertEq("Log file line cnt", CountLines(path), 2);
    AssertFileRegexEq("Log file contents", path, DEFAULT_LOG_FILE_PATTERN);
}


int g_iCustomErrCnt = 0;

void TestCustomErrorHandler()
{
    SetTestContext("Test Custom Error Handler");

    g_iCustomErrCnt = 0;

    char path[PLATFORM_MAX_PATH];
    path = PrepareTestPath("err-handler/custom_handler.log");

    Logger logger = BasicFileSink.CreateLogger(LOGGER_NAME, path);
    logger.SetPattern("%v");
    logger.FlushOn(LogLevel_Info);
    logger.SetErrorHandler(CustomErrorHandler);

    logger.InfoEx("Test message %d", 1);
    logger.InfoEx("Test message %d", 2);
    logger.InfoEx("Test message %d %d", 3);
    logger.InfoEx("Test message %d %d", 4);
    logger.InfoEx("Test message %d %d", 5);
    logger.InfoEx("Test message %d", 6);
    logger.InfoEx("Test message %d", 7);
    delete logger;

    AssertEq("Custom handler cnt", g_iCustomErrCnt, 3);
    AssertEq("Log file line cnt", CountLines(path), 4);
    AssertFileRegexEq("Log file contents", path, CUSTOM_LOG_FILE_PATTERN);
}



void CustomErrorHandler(const char[] msg, const char[] name, const char[] file, int line, const char[] func)
{
    g_iCustomErrCnt++;

    AssertStrRegexEq("CB msg pattern", msg, ERR_MSG_PATTERN);
    AssertStrEq("CB name", name, LOGGER_NAME);
    AssertStrRegexEq("CB file pattern", file, PLUGIN_PATTERN);
    AssertStrEq("CB func", func, "TestCustomErrorHandler");
}

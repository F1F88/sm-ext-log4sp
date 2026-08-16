#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <log4sp>

#include "../test_utils"


#define LOGGER_NAME                 "test-err-handler"


public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_logger_err_handler", Command_Test);
}

Action Command_Test(int args)
{
    PrintToServer("---- START TEST LOGGER ERR HANDLER ----");

    PrepareTestPath("err-handler/");

    TestDefaultErrorHandler();

    TestCustomErrorHandler();

    PrintToServer("---- STOP TEST LOGGER ERR HANDLER ----");
    return Plugin_Handled;
}


void TestDefaultErrorHandler()
{
    SetTestContext("Test Default Error Handler");

    MarkErrorTestStart("Test Default Error Handler");

    char path[PLATFORM_MAX_PATH];
    BuildTestPath(path, sizeof(path), "err-handler/default_handler.log");

    BasicFileSink sink = new BasicFileSink(path);
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);
    logger.SetPattern("%v");

    logger.InfoF("Test message %d", 1);
    logger.InfoF("Test message %d %d", 2);
    logger.InfoF("Test message %d", 3);
    delete logger;
    delete sink;

    MarkErrorTestEnd("Test Default Error Handler");

    AssertEq("Skip param format error log, count lines", CountLines(path), 2);
    AssertFileMatch("Skip param format error log, contents match", path, "Test message 1" ... P_EOL ... "Test message 3" ... P_EOL);

    // AssertEq("Default error handler log to SM file, count lines", CountLines(GetErrorFilename()), 3);
    char[] pattern = "L [0-9]{2}/[0-9]{2}/[0-9]{4} - [0-9]{2}:[0-9]{2}:[0-9]{2}: \\[.*\\] \\[test-logger-err-handler.sp::[0-9]+\\] \\[test-err-handler\\] String formatted incorrectly - parameter 4 \\(total 3\\)(\n|\r\n)";
    AssertFileMatch("Default error handler log to SM file, contents match", GetErrorFilename(), pattern);

    // 若检验通过，则删除本次测试生成的日志信息以保持 SM 错误日志的简洁
    // DeleteFile(GetErrorFilename());
}


int g_iCustomErrCnt = 0;

void TestCustomErrorHandler()
{
    SetTestContext("Test Custom Error Handler");

    g_iCustomErrCnt = 0;

    char path[PLATFORM_MAX_PATH];
    BuildTestPath(path, sizeof(path), "err-handler/custom_handler.log");

    BasicFileSink sink = new BasicFileSink(path);
    Logger logger = new Logger(LOGGER_NAME);
    logger.AddSink(sink);
    logger.SetPattern("%v");
    logger.FlushOn(LogLevel_Info);
    logger.SetErrorHandler(null, CustomErrorHandler);

    logger.InfoF("Test message %d", 1);
    logger.InfoF("Test message %d", 2);
    logger.InfoF("Test message %d %d", 3);
    logger.InfoF("Test message %d %d", 4);
    logger.InfoF("Test message %d %d", 5);
    logger.InfoF("Test message %d", 6);
    logger.InfoF("Test message %d", 7);
    delete logger;
    delete sink;

    AssertEq("Call custom error handler count", g_iCustomErrCnt, 3);
    AssertEq("Skip param format error log, count lines", CountLines(path), 4);
    AssertFileMatch("Skip param format error log, contents match", path, "Test message 1" ... P_EOL ... "Test message 2" ... P_EOL ... "Test message 6" ... P_EOL ... "Test message 7" ... P_EOL);
}



void CustomErrorHandler(const char[] origin, const SourceLoc loc, const char[] msg)
{
    g_iCustomErrCnt++;

    AssertStrEq("OnCustomErrorHandler msg", msg, "String formatted incorrectly - parameter 4 (total 3)");
    AssertStrEq("OnCustomErrorHandler origin", origin, LOGGER_NAME);
    AssertStrMatch("OnCustomErrorHandler file match", loc.filename, ".*test-logger-err-handler.sp");
    AssertStrEq("OnCustomErrorHandler func", loc.funcname, "TestCustomErrorHandler");
}

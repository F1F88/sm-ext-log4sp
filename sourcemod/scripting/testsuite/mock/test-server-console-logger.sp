#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <log4sp>


public void OnPluginStart()
{
    Test();
    RegServerCmd("sm_log4sp_test_server_console_logger", Command_Test);
}

Action Command_Test(int args)
{
    Test();
    return Plugin_Handled;
}


void Test()
{
    PrintToServer("------ Started testing Server-Console-Logger -----");

    TestServerConsole();

    PrintToServer("-------- Test Server-Console-Logger ended --------");
}

void TestServerConsole()
{
    ServerConsoleSink sink = new ServerConsoleSink();
    Logger logger = new Logger("test-server-console");
    logger.SetLevel(LogLevel_Trace);
    logger.AddSink(sink);
    SinkCleanupAndDelete(sink);

    logger.Trace("Test server console 1");
    logger.Debug("Test server console 2");
    logger.Log(LogLevel_Info, "Test server console 3");
    logger.LogF(LogLevel_Warn, "Test server console %d", 4);
    logger.LogSrc(LogLevel_Error, "Test server console 5");
    logger.LogSrcF(LogLevel_Fatal, "Test server console %d", 6);

    LoggerCleanupAndDelete(logger);
}

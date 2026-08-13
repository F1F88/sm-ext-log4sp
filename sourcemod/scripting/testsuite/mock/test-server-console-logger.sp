#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <log4sp>


public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_server_console_logger", Command_Test);
}

Action Command_Test(int args)
{
    PrintToServer("---- START TEST SERVER CONSOLE LOGGER ----");

    TestServerConsole();

    PrintToServer("---- STOP TEST SERVER CONSOLE LOGGER ----");
    return Plugin_Handled;
}


void TestServerConsole()
{
    ServerConsoleSink sink = new ServerConsoleSink();
    Logger logger = new Logger("test-server-console");
    logger.AddSink(sink);
    logger.SetPattern("%+");
    logger.SetLevel(LogLevel_Trace);

    logger.Trace("Test server console");
    logger.Debug("Test server console");
    logger.Info("Test server console");
    logger.Warn("Test server console");
    logger.Error("Test server console");
    logger.Fatal("Test server console");

    delete logger;
    delete sink;
}

#include <testing>

#include <log4sp>

#pragma semicolon 1
#pragma newdecls required


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
    SetTestContext("Test server cosnole");

    Logger logger = ServerConsoleSink.CreateLogger("test-server-console");
    logger.SetPattern("%+");
    logger.SetLevel(LogLevel_Trace);

    logger.Trace("Test server console");
    logger.Debug("Test server console");
    logger.Info("Test server console");
    logger.Warn("Test server console");
    logger.Error("Test server console");
    logger.Fatal("Test server console");

    delete logger;
}

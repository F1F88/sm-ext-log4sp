#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <log4sp>


public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_tcp_logger", Command_Test);
}

Action Command_Test(int args)
{
    PrintToServer("---- START TEST TCP LOGGER ----");

    TestTCPSink();

    PrintToServer("---- STOP TEST TCP LOGGER ----");
    return Plugin_Handled;
}


void TestTCPSink()
{
    Logger logger = TCPSink.CreateLogger("test-tcp-logger", "127.0.0.1", 27001);
    logger.SetLevel(LogLevel_Trace);

    logger.Trace("Test tcp message");
    logger.Debug("Test tcp message");
    logger.Info("Test tcp message");
    logger.Warn("Test tcp message");
    logger.Error("Test tcp message");
    logger.Fatal("Test tcp message");

    delete logger;
}

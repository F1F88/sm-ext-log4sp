#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <log4sp>


public void OnPluginStart()
{
    RegServerCmd("sm_log4sp_test_udp_logger", Command_Test);
}

Action Command_Test(int args)
{
    PrintToServer("---- START TEST UDP LOGGER ----");

    TestUDPSink();

    PrintToServer("---- STOP TEST UDP LOGGER ----");
    return Plugin_Handled;
}


void TestUDPSink()
{
    Logger logger = UDPSink.CreateLogger("test-udp-logger", "127.0.0.1", 27002);
    logger.SetLevel(LogLevel_Trace);

    logger.Trace("Test UDP message");
    logger.Debug("Test UDP message");
    logger.Info("Test UDP message");
    logger.Warn("Test UDP message");
    logger.Error("Test UDP message");
    logger.Fatal("Test UDP message");

    delete logger;
}

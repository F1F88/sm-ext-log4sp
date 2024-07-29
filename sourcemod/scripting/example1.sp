#include <sourcemod>
#include <log4sp>

Logger myLogger;
public void OnPluginStart()
{
    myLogger = Logger.CreateServerConsoleLogger("myLogger1");
    myLogger.Info("===== Example1 code initialization is complete! =====");
}

public void OnClientPutInServer(int client)
{
	myLogger.InfoAmxTpl("Client %L put in server.", client);
}

#include <sourcemod>

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_NAME                         "Test Log4sp"
#define PLUGIN_AUTHOR                       "F1F88"
#define PLUGIN_VERSION                      "v1.0.0"
#define PLUGIN_DESCRIPTION                  "Test Logging for SourcePawn extension"
#define PLUGIN_URL                          "https://github.com/F1F88/sm-ext-log4sp"

public Plugin myinfo = {
    name        = PLUGIN_NAME,
    author      = PLUGIN_AUTHOR,
    description = PLUGIN_DESCRIPTION,
    version     = PLUGIN_VERSION,
    url         = PLUGIN_URL
};

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

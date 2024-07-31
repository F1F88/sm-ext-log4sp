#include <sourcemod>

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_NAME                         "Log4sp Example 1"
#define PLUGIN_AUTHOR                       "F1F88"
#define PLUGIN_VERSION                      "v1.0.0"
#define PLUGIN_DESCRIPTION                  "Logging for SourcePawn example 1"
#define PLUGIN_URL                          "https://github.com/F1F88/sm-ext-log4sp"

public Plugin myinfo = {
    name        = PLUGIN_NAME,
    author      = PLUGIN_AUTHOR,
    description = PLUGIN_DESCRIPTION,
    version     = PLUGIN_VERSION,
    url         = PLUGIN_URL
};
#include <sdktools>
#include <log4sp>

Logger myLogger;
public void OnPluginStart()
{
    myLogger = Logger.CreateServerConsoleLogger("logger-example-1");

    RegConsoleCmd("sm_log4sp_example1", CommandCallback);

    myLogger.Debug("===== Example 1 code initialization is complete! =====");
}

/**
 * Get client aiming entity info.
 * 获取玩家瞄准的实体信息
 */
Action CommandCallback(int client, int args)
{
    if (client <= 0 || client > MaxClients || !IsClientInGame(client))
    {
        myLogger.Info("[SM] Command is in-game only.");
        return Plugin_Handled;
    }

    int entity = GetClientAimTarget(client, false);
    if (entity == -2)
    {
        myLogger.Fatal("[SM] The GetClientAimTarget() function is not supported.");
        return Plugin_Handled;
    }

    if (entity == -1)
    {
        myLogger.Warn("[SM] No entity is being aimed at.");
        return Plugin_Handled;
    }

    if (!IsValidEntity(entity))
    {
        myLogger.ErrorAmxTpl("[SM] entity %d is invalid.", entity);
        return Plugin_Handled;
    }

    char classname[64];
    GetEntityClassname(entity, classname, sizeof(classname));
    myLogger.InfoAmxTpl("[SM] The client %L is aiming a (%d) %s entity.", client, entity, classname);

    return Plugin_Handled;
}

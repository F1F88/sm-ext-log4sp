#include <sourcemod>

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_NAME                         "Log4sp Example 1"
#define PLUGIN_AUTHOR                       "F1F88"
#define PLUGIN_VERSION                      "v1.2.0"
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
    // Default LogLevel: LogLevel_Info
    // Default Pattern: [%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v
    myLogger = Logger.CreateServerConsoleLogger("logger-example-1");

    RegConsoleCmd("sm_log4sp_example1", CommandCallback);

    // Debug is lower than the default LogLevel, so this line of code won't output log message
    // Debug 小于默认日志级别 Info, 所以这行代码不会输出日志消息
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
        // [2024-08-01 12:34:56.789] [logger-example-1] [info] Command is in-game only.
        myLogger.Info("Command is in-game only.");
        return Plugin_Handled;
    }

    int entity = GetClientAimTarget(client, false);
    if (entity == -2)
    {
        // [2024-08-01 12:34:56.789] [logger-example-1] [fatal] The GetClientAimTarget() function is not supported.
        myLogger.Fatal("The GetClientAimTarget() function is not supported.");
        return Plugin_Handled;
    }

    if (entity == -1)
    {
        // [2024-08-01 12:34:56.789] [logger-example-1] [warn] client (1) is not aiming at entity.
        myLogger.WarnAmxTpl("client (%d) is not aiming at entity.", client);
        return Plugin_Handled;
    }

    if (!IsValidEntity(entity))
    {
        // [2024-08-01 12:34:56.789] [logger-example-1] [error] entity (444) is invalid.
        myLogger.ErrorAmxTpl("entity (%d) is invalid.", entity);
        return Plugin_Handled;
    }

    char classname[64];
    GetEntityClassname(entity, classname, sizeof(classname));

    // [2024-08-01 12:34:56.789] [logger-example-1] [info] client (1) is aiming a (403 - prop_door_breakable) entity.
    myLogger.InfoAmxTpl("client (%d) is aiming a (%d - %s) entity.", client, entity, classname);

    return Plugin_Handled;
}

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_VERSION "2.0.0"

public Plugin myinfo =
{
    name = "[Any] Log4sp Manager",
    author = "blueblur & F1F88",
    description = "Helper plugin to manage extension for log4sp",
    version = PLUGIN_VERSION,
    url = "https://github.com/F1F88/sm-ext-log4sp"
}


#include <sourcemod>

#include "log4sp-manager/command.sp"
#include "log4sp-manager/menu.sp"
#include "log4sp-manager/registry.sp"


public APLRes AskPluginLoad2(Handle myself, bool late, char[] error, int err_max)
{
    CreateRegistryNatives();
    return APLRes_Success;
}

public void OnPluginStart()
{
    LoadTranslations("common.phrases");

    InitializeRegistryMembers();

    RegisterCommandCommands();
    RegisterMenuCommands();

    RegPluginLibrary("log4sp_manager");

    CreateConVar("log4sp_manager_version", PLUGIN_VERSION, "Version of the helper plugin log4sp manager.", FCVAR_SPONLY | FCVAR_NOTIFY | FCVAR_DONTRECORD);

#if !defined NDEBUG
    PrintToServer("[SM] Log4sp Manager " ... PLUGIN_VERSION ... " initialize complete!");
    ServerCommand("sm_log4sp version");
#endif
}

public void OnNotifyPluginUnloaded(Handle plugin)
{
    RegistryHandleOwnershipPatch(plugin);
}

stock bool IsValidClient(int client)
{
    return client > 0 && client <= MaxClients && IsClientInGame(client);
}

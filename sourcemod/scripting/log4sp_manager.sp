#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <log4sp>


#define PLUGIN_VERSION "v1.0.0"

public Plugin myinfo =
{
    name = "[Any] Log4sp Manager",
    author = "blueblur",
    description = "Helper plugin to manage extension for log4sp",
    version = PLUGIN_VERSION,
    url = "https://github.com/F1F88/sm-ext-log4sp"
}

public void OnPluginStart()
{
    CreateConVar("log4sp_manager_version", PLUGIN_VERSION, "Version of the helper plugin log4sp manager.", FCVAR_SPONLY | FCVAR_NOTIFY | FCVAR_DONTRECORD);

    RegAdminCmd("sm_log4sp", Cmd_Log4sp, ADMFLAG_CONFIG, "Forward commands");
    RegAdminCmd("sm_log4sp_menu", Cmd_Log4spMenu, ADMFLAG_CONFIG, "Displays the log4sp manager menu");
    RegAdminCmd("sm_log4sp_manager", Cmd_Log4spMenu, ADMFLAG_CONFIG, "Displays the log4sp manager menu");
    RegAdminCmd("sm_log4sp_manager_menu", Cmd_Log4spMenu, ADMFLAG_CONFIG, "Displays the log4sp manager menu");

    LoadTranslations("common.phrases");
}


Action Cmd_Log4sp(int client, int args)
{
    if (!IsValidClient(client))
    {
        ReplyToCommand(client, "[SM] %t", "Command is in-game only");
        return Plugin_Handled;
    }

    char command[256];
    GetCmdArgString(command, sizeof(command));

    // Forward commands to the server
    char result[1024];
    ServerCommandEx(result, sizeof(result), "sm log4sp %s", command);

#if SOURCEMOD_V_MINOR <= 11
    bool antiFlood = !command[0] || !strncmp(command, "list", 4) || !strncmp(command, "apply_all", 9);
#else
    bool antiFlood = !command[0] || !strncmp(command, "list", sizeof("list") - 1) || !strncmp(command, "apply_all", sizeof("apply_all") - 1);
#endif

    // Preventing flooded chat
    if (GetCmdReplySource() == SM_REPLY_TO_CHAT && antiFlood)
    {
        ReplyToCommand(client, "[SM] %t", "See console for output");
        PrintToConsole(client, result);
    }
    else
    {
        ReplyToCommand(client, result);
    }

    // TODO: 是否可以合并管理菜单指令？
    // DisplayManagerMenu(client);

    return Plugin_Handled;
}

Action Cmd_Log4spMenu(int client, int args)
{
    if (!IsValidClient(client))
    {
        ReplyToCommand(client, "[SM] %t", "Command is in-game only");
        return Plugin_Handled;
    }

    DisplayManagerMenu(client);

    return Plugin_Handled;
}


void DisplayManagerMenu(int client, int time = MENU_TIME_FOREVER)
{
    if (!IsValidClient(client))
        return;

    ArrayList names = new ArrayList(ByteCountToCells(64));
    Logger.ApplyAll(ApplyAllLogger_GetNames, names);

    Menu menu = new Menu(MenuHandler_Manager);
    menu.SetTitle("Select Logger");

    menu.AddItem("", "All");
    char name[64];
    for (int i = 0; i < names.Length; i++)
    {
        names.GetString(i, name, sizeof(name));
        menu.AddItem(name, name);
    }

    delete names;
    menu.Display(client, time);
}

void DisplayApplyAllMenu(int client, int time = MENU_TIME_FOREVER)
{
    if (!IsValidClient(client))
        return;

    Menu menu = new Menu(MenuHandler_ApplyAll);
    menu.SetTitle("Select a function");

    menu.AddItem("1", "Sets the log level");
    menu.AddItem("2", "Flush");
    menu.AddItem("3", "Sets the flush level");

    menu.ExitBackButton = true;
    menu.Display(client, time);
}

void DisplayApplyAllSetLogLvlMenu(int client, int time = MENU_TIME_FOREVER)
{
    if (!IsValidClient(client))
        return;

    Menu menu = new Menu(MenuHandler_AllSetLogLvl);
    menu.SetTitle("Select a log level");

    menu.AddItem(LOG4SP_LEVEL_NAME_TRACE, "trace");
    menu.AddItem(LOG4SP_LEVEL_NAME_DEBUG, "debug");
    menu.AddItem(LOG4SP_LEVEL_NAME_INFO, "info");
    menu.AddItem(LOG4SP_LEVEL_NAME_WARN, "warn");
    menu.AddItem(LOG4SP_LEVEL_NAME_ERROR, "error");
    menu.AddItem(LOG4SP_LEVEL_NAME_FATAL, "fatal");
    menu.AddItem(LOG4SP_LEVEL_NAME_OFF, "off");

    menu.ExitBackButton = true;
    menu.Display(client, time);
}

void DisplayApplyAllSetFlushLvlMenu(int client, int time = MENU_TIME_FOREVER)
{
    if (!IsValidClient(client))
        return;

    Menu menu = new Menu(MenuHandler_AllSetFlushLvl);
    menu.SetTitle("Select a flush level");

    menu.AddItem(LOG4SP_LEVEL_NAME_TRACE, "trace");
    menu.AddItem(LOG4SP_LEVEL_NAME_DEBUG, "debug");
    menu.AddItem(LOG4SP_LEVEL_NAME_INFO, "info");
    menu.AddItem(LOG4SP_LEVEL_NAME_WARN, "warn");
    menu.AddItem(LOG4SP_LEVEL_NAME_ERROR, "error");
    menu.AddItem(LOG4SP_LEVEL_NAME_FATAL, "fatal");
    menu.AddItem(LOG4SP_LEVEL_NAME_OFF, "off");

    menu.ExitBackButton = true;
    menu.Display(client, time);
}

void DisplayLoggerMenu(const char[] name, int client, int time = MENU_TIME_FOREVER)
{
    if (!IsValidClient(client))
        return;

    Logger logger = Logger.Get(name);
    if (!logger)
        return;

    // 1.8.0 的 LogLevelToName 不向下兼容, 所以这里用个怪招, 顺带提高性能
    static const char lvlNames[][] = {LOG4SP_LEVEL_NAME_TRACE, LOG4SP_LEVEL_NAME_DEBUG,
                                      LOG4SP_LEVEL_NAME_INFO, LOG4SP_LEVEL_NAME_WARN,
                                      LOG4SP_LEVEL_NAME_ERROR, LOG4SP_LEVEL_NAME_FATAL,
                                      LOG4SP_LEVEL_NAME_OFF};

    Menu menu = new Menu(MenuHandler_Logger);

    int logLvlIdx = view_as<int>(logger.GetLevel());
    int flushLvlIdx = view_as<int>(logger.GetFlushLevel());
    menu.SetTitle("Logger %s\nLog level: %s\nFlush level: %s\n ", name, lvlNames[logLvlIdx], lvlNames[flushLvlIdx]);

    menu.AddItem("1", "Set log level");
    menu.AddItem("2", "Flush");
    menu.AddItem("3", "Set flush level");
    menu.AddItem(name, "", ITEMDRAW_IGNORE); // Pass logger name

    menu.ExitBackButton = true;
    menu.Display(client, time);
}

void DisplayLoggerSetLogLvlMenu(const char[] name, int client, int time = MENU_TIME_FOREVER)
{
    if (!IsValidClient(client))
        return;

    Logger logger = Logger.Get(name);
    if (!logger)
        return;

    Menu menu = new Menu(MenuHandler_SetLogLvl);
    menu.SetTitle("Select a log level for %s", name);

    menu.AddItem(LOG4SP_LEVEL_NAME_TRACE, "trace");
    menu.AddItem(LOG4SP_LEVEL_NAME_DEBUG, "debug");
    menu.AddItem(LOG4SP_LEVEL_NAME_INFO, "info");
    menu.AddItem(LOG4SP_LEVEL_NAME_WARN, "warn");
    menu.AddItem(LOG4SP_LEVEL_NAME_ERROR, "error");
    menu.AddItem(LOG4SP_LEVEL_NAME_FATAL, "fatal");
    menu.AddItem(LOG4SP_LEVEL_NAME_OFF, "off");
    menu.AddItem(name, "", ITEMDRAW_IGNORE); // Pass logger name

    menu.ExitBackButton = true;
    menu.Display(client, time);
}

void DisplayLoggerSetFlushLvlMenu(const char[] name, int client, int time = MENU_TIME_FOREVER)
{
    if (!IsValidClient(client))
        return;

    Logger logger = Logger.Get(name);
    if (!logger)
        return;

    Menu menu = new Menu(MenuHandler_SetFlushLvl);
    menu.SetTitle("Select a flush level for %s", name);

    menu.AddItem(LOG4SP_LEVEL_NAME_TRACE, "trace");
    menu.AddItem(LOG4SP_LEVEL_NAME_DEBUG, "debug");
    menu.AddItem(LOG4SP_LEVEL_NAME_INFO, "info");
    menu.AddItem(LOG4SP_LEVEL_NAME_WARN, "warn");
    menu.AddItem(LOG4SP_LEVEL_NAME_ERROR, "error");
    menu.AddItem(LOG4SP_LEVEL_NAME_FATAL, "fatal");
    menu.AddItem(LOG4SP_LEVEL_NAME_OFF, "off");
    menu.AddItem(name, "", ITEMDRAW_IGNORE); // Pass logger name

    menu.ExitBackButton = true;
    menu.Display(client, time);
}

#if SOURCEMOD_V_MINOR <= 11
int MenuHandler_Manager(Menu menu, MenuAction action, int param1, int param2)
#else
void MenuHandler_Manager(Menu menu, MenuAction action, int param1, int param2)
#endif
{
    switch (action)
    {
        case MenuAction_Select:
        {
            if (param2 == 0)
            {
                DisplayApplyAllMenu(param1);
            }
            else
            {
                char name[64];
                menu.GetItem(param2, name, sizeof(name));
                if (!Logger.Get(name))
                {
                    PrintToChat(param1, "[SM] Logger with \"%s\" not exists.", name);
#if SOURCEMOD_V_MINOR <= 11
                    return 0;
#else
                    return;
#endif
                }

                DisplayLoggerMenu(name, param1);
            }
        }
        case MenuAction_End:
        {
            delete menu;
        }
    }
#if SOURCEMOD_V_MINOR <= 11
    return 0;
#endif
}

#if SOURCEMOD_V_MINOR <= 11
int MenuHandler_ApplyAll(Menu menu, MenuAction action, int param1, int param2)
#else
void MenuHandler_ApplyAll(Menu menu, MenuAction action, int param1, int param2)
#endif
{
    switch (action)
    {
        case MenuAction_Select:
        {
            switch (param2)
            {
                case 0:
                {
                    DisplayApplyAllSetLogLvlMenu(param1);
                }
                case 1:
                {
                    PrintToChat(param1, "[SM] All loggers will flush its contents.");
                    Logger.ApplyAll(ApplyAllLogger_FlushAll);
                }
                case 2:
                {
                    DisplayApplyAllSetFlushLvlMenu(param1);
                }
            }
        }
        case MenuAction_End:
        {
            delete menu;
        }
        case MenuAction_Cancel:
        {
            if (param2 == MenuCancel_ExitBack)
                DisplayManagerMenu(param1);
        }
    }
#if SOURCEMOD_V_MINOR <= 11
    return 0;
#endif
}

#if SOURCEMOD_V_MINOR <= 11
int MenuHandler_AllSetLogLvl(Menu menu, MenuAction action, int param1, int param2)
#else
void MenuHandler_AllSetLogLvl(Menu menu, MenuAction action, int param1, int param2)
#endif
{
    switch (action)
    {
        case MenuAction_Select:
        {
            char level[32];
            menu.GetItem(param2, level, sizeof(level));
            PrintToChat(param1, "[SM] All loggers will set log level to '%s'.", level);
            Logger.ApplyAll(ApplyAllLogger_SetLevel, NameToLogLevel(level));
        }
        case MenuAction_End:
        {
            delete menu;
        }
        case MenuAction_Cancel:
        {
            if (param2 == MenuCancel_ExitBack)
                DisplayApplyAllMenu(param1);
        }
    }
#if SOURCEMOD_V_MINOR <= 11
    return 0;
#endif
}

#if SOURCEMOD_V_MINOR <= 11
int MenuHandler_AllSetFlushLvl(Menu menu, MenuAction action, int param1, int param2)
#else
void MenuHandler_AllSetFlushLvl(Menu menu, MenuAction action, int param1, int param2)
#endif
{
    switch (action)
    {
        case MenuAction_Select:
        {
            char level[32];
            menu.GetItem(param2, level, sizeof(level));
            PrintToChat(param1, "[SM] All loggers will set flush level to '%s'.", level);
            Logger.ApplyAll(ApplyAllLogger_SetFlushLevel, NameToLogLevel(level));
        }
        case MenuAction_End:
        {
            delete menu;
        }
        case MenuAction_Cancel:
        {
            if (param2 == MenuCancel_ExitBack)
                DisplayApplyAllMenu(param1);
        }
    }
#if SOURCEMOD_V_MINOR <= 11
    return 0;
#endif
}

#if SOURCEMOD_V_MINOR <= 11
int MenuHandler_Logger(Menu menu, MenuAction action, int param1, int param2)
#else
void MenuHandler_Logger(Menu menu, MenuAction action, int param1, int param2)
#endif
{
    switch (action)
    {
        case MenuAction_Select:
        {
            const int nameIndex = 3;
            char name[64];
            menu.GetItem(nameIndex, name, sizeof(name));
            Logger logger = Logger.Get(name);
            if (!logger)
            {
                PrintToChat(param1, "[SM] Logger with \"%s\" not exists.", name);
#if SOURCEMOD_V_MINOR <= 11
                return 0;
#else
                return;
#endif
            }

            switch (param2)
            {
                case 0:
                {
                    DisplayLoggerSetLogLvlMenu(name, param1);
                }
                case 1:
                {
                    PrintToChat(param1, "[SM] Logger '%s' will flush its contents.", name);
                    logger.Flush();
                }
                case 2:
                {
                    DisplayLoggerSetFlushLvlMenu(name, param1);
                }
            }
        }
        case MenuAction_End:
        {
            delete menu;
        }
        case MenuAction_Cancel:
        {
            if (param2 == MenuCancel_ExitBack)
                DisplayManagerMenu(param1);
        }
    }
#if SOURCEMOD_V_MINOR <= 11
    return 0;
#endif
}


#if SOURCEMOD_V_MINOR <= 11
int MenuHandler_SetLogLvl(Menu menu, MenuAction action, int param1, int param2)
#else
void MenuHandler_SetLogLvl(Menu menu, MenuAction action, int param1, int param2)
#endif
{
    switch (action)
    {
        case MenuAction_Select:
        {
            const int nameIndex = view_as<int>(LogLevel_Total);
            char name[64];
            menu.GetItem(nameIndex, name, sizeof(name));
            Logger logger = Logger.Get(name);
            if (!logger)
            {
                PrintToChat(param1, "[SM] Logger with \"%s\" not exists.", name);
#if SOURCEMOD_V_MINOR <= 11
                return 0;
#else
                return;
#endif
            }

            char level[32];
            menu.GetItem(param2, level, sizeof(level));

            PrintToChat(param1, "[SM] Logger '%s' will set log level to '%s'.", name, level);
            logger.SetLevel(NameToLogLevel(level));
        }
        case MenuAction_End:
        {
            delete menu;
        }
        case MenuAction_Cancel:
        {
            if (param2 == MenuCancel_ExitBack)
                DisplayManagerMenu(param1);
        }
    }
#if SOURCEMOD_V_MINOR <= 11
    return 0;
#endif
}


#if SOURCEMOD_V_MINOR <= 11
int MenuHandler_SetFlushLvl(Menu menu, MenuAction action, int param1, int param2)
#else
void MenuHandler_SetFlushLvl(Menu menu, MenuAction action, int param1, int param2)
#endif
{
    switch (action)
    {
        case MenuAction_Select:
        {
            const int nameIndex = view_as<int>(LogLevel_Total);
            char name[64];
            menu.GetItem(nameIndex, name, sizeof(name));
            Logger logger = Logger.Get(name);
            if (!logger)
            {
                PrintToChat(param1, "[SM] Logger with \"%s\" not exists.", name);
#if SOURCEMOD_V_MINOR <= 11
                return 0;
#else
                return;
#endif
            }

            char level[32];
            menu.GetItem(param2, level, sizeof(level));

            PrintToChat(param1, "[SM] Logger '%s' will set flush level to '%s'.", name, level);
            logger.FlushOn(NameToLogLevel(level));
        }
        case MenuAction_End:
        {
            delete menu;
        }
        case MenuAction_Cancel:
        {
            if (param2 == MenuCancel_ExitBack)
                DisplayManagerMenu(param1);
        }
    }
#if SOURCEMOD_V_MINOR <= 11
    return 0;
#endif
}

// array have at least one logger called LOG4SP_GLOBAL_LOGGER_NAME created by extension
void ApplyAllLogger_GetNames(Logger logger, ArrayList array)
{
    char buffer[64];
    logger.GetName(buffer, sizeof(buffer));
    array.PushString(buffer);
}

void ApplyAllLogger_FlushAll(Logger logger)
{
    logger.Flush();
}

void ApplyAllLogger_SetLevel(Logger logger, LogLevel level)
{
    logger.SetLevel(level);
}

void ApplyAllLogger_SetFlushLevel(Logger logger, LogLevel level)
{
    logger.FlushOn(level);
}

stock bool IsValidClient(int client)
{
    return client > 0 && client <= MaxClients && IsClientInGame(client);
}

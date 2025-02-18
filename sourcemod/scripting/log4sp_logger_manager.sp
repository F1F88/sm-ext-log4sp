#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <log4sp>

#define PLUGIN_VERSION "v1.1.0"

public Plugin myinfo =
{
	name = "[Any] Logger Manager for Log4sp",
	author = "blueblur",
	description = "Instance manager for log4sp.",
	version = PLUGIN_VERSION,
	url = "https://github.com/F1F88/sm-ext-log4sp"
}

public void OnPluginStart()
{
    CreateConVar("log4sp_logger_manager_version", PLUGIN_VERSION, "The version of the Logger Manager plugin.", FCVAR_NOTIFY | FCVAR_DONTRECORD);

    RegAdminCmd("sm_log4sp", Cmd_Logger, ADMFLAG_CONFIG, "Open logger menu.");
}

public Action OnClientSayCommand(int client, const char[] command, const char[] sArgs)
{
    // do not listen server console.
    if (!client) return Plugin_Continue;

    if (!!strncmp(sArgs, "sm log4sp", sizeof("sm log4sp") - 1))
        return Plugin_Continue;
    {
        char sBuffer[1024];
        ServerCommandEx(sBuffer, sizeof(sBuffer), sArgs);
        if (!strcmp(sArgs, "sm log4sp") || !strcmp(sArgs, "sm log4sp list"))
        {
            PrintToChat(client, "[Log4sp] See console for the output.");
            PrintToConsole(client, "%s", sBuffer);
        }
        else
        {
            PrintToChat(client, "%s", sBuffer);
        }

        // block this message from spamming public chat, the others won't see it.
        return Plugin_Handled;
    }

    return Plugin_Continue;
}

Action Cmd_Logger(int client, int args)
{
    if (!client)
    {
        ReplyToCommand(client, "[Log4sp] This command can only be used in-game.");
        return Plugin_Handled;
    }

    CreateLoggerListMenu(client);
    return Plugin_Handled;
}

void CreateLoggerListMenu(int client)
{
    ArrayList hArray = new ArrayList(ByteCountToCells(64));
    Logger.ApplyAll(ApplyAllLogger_GetAllLoggerName, hArray);

    Menu menu = new Menu(MenuHandler_LoggerList);
    menu.SetTitle("Select a logger to manager:");
    menu.AddItem("", "Apply to all loggers");

    for (int i = 0; i < hArray.Length; i++)
    {
        char sName[64];
        hArray.GetString(i, sName, sizeof(sName));
        menu.AddItem(sName, sName);
    }

    delete hArray;
    menu.Display(client, MENU_TIME_FOREVER);
}

// hArray have at least one logger called "log4sp" created by extension.
void ApplyAllLogger_GetAllLoggerName(Logger logger, ArrayList hArray)
{
    char buffer[64];
    logger.GetName(buffer, sizeof(buffer));
    hArray.PushString(buffer);
}

void MenuHandler_LoggerList(Menu menu, MenuAction action, int param1, int param2)
{
    switch (action)
    {
        case MenuAction_Select:
        {
            if (param2 == 0)
            {
                Menu submenu = new Menu(MenuHandler_ApplyToAll);
                submenu.SetTitle("Choose an action to apply to all loggers:");
                submenu.AddItem("1", "Set all logger's level");
                submenu.AddItem("2", "Flush all loggers");
                submenu.AddItem("3", "Set all logger's flush level");
                submenu.ExitBackButton = true;
                submenu.Display(param1, MENU_TIME_FOREVER);
            }
            else
            {
                char sInfo[64];
                menu.GetItem(param2, sInfo, sizeof(sInfo));
                Logger logger = Logger.Get(sInfo);
                if (!logger)
                {
                    PrintToChat(param1, "[Log4sp] Logger '%s' not found.", sInfo);
                    return;
                }

                char sBuffer[64];
                Menu submenu = new Menu(MenuHandler_ManageLogger);
                submenu.SetTitle("Manage Logger: %s", sInfo);
                submenu.AddItem(sInfo, "", ITEMDRAW_IGNORE);

                LogLevelToName(logger.GetLevel(), sBuffer, sizeof(sBuffer));
                Format(sBuffer, sizeof(sBuffer), "Current log level: %s", sBuffer);
                submenu.AddItem("", sBuffer, ITEMDRAW_DISABLED);

                LogLevelToName(logger.GetFlushLevel(), sBuffer, sizeof(sBuffer));
                Format(sBuffer, sizeof(sBuffer), "Current flush level: %s", sBuffer);
                submenu.AddItem("", sBuffer, ITEMDRAW_DISABLED);

                submenu.AddItem("1", "Set log level");
                submenu.AddItem("2", "Flush");
                submenu.AddItem("3", "Set flush level");
                submenu.ExitBackButton = true;
                submenu.Display(param1, MENU_TIME_FOREVER);
            }
        }

        case MenuAction_End:
            delete menu;
    }
}

void MenuHandler_ApplyToAll(Menu menu, MenuAction action, int param1, int param2)
{
    switch (action)
    {
        case MenuAction_Select:
        {
            switch (param2)
            {
                case 0:
                {
                    Menu submenu = new Menu(MenuHandler_SetAllLoggerLevel);
                    submenu.SetTitle("Set log level for all loggers:");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_OFF, "off");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_FATAL, "fatal");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_ERROR, "error");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_WARN, "warn");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_INFO, "info");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_DEBUG, "debug");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_TRACE, "trace");
                    submenu.ExitBackButton = true;
                    submenu.Display(param1, MENU_TIME_FOREVER);
                }

                case 1:
                {
                    Logger.ApplyAll(ApplyAllLogger_FlushAll);
                    PrintToChat(param1, "[Log4sp] All loggers have been flushed.");
                }

                case 2:
                {
                    Menu submenu = new Menu(MenuHandler_SetAllLoggerFlushLevel);
                    submenu.SetTitle("Set flush level for all loggers:");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_OFF, "off");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_FATAL, "fatal");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_ERROR, "error");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_WARN, "warn");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_INFO, "info");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_DEBUG, "debug");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_TRACE, "trace");
                    submenu.ExitBackButton = true;
                    submenu.Display(param1, MENU_TIME_FOREVER);
                }
            }
        }

        case MenuAction_End:
            delete menu;

        case MenuAction_Cancel:
        {
            if (param2 == MenuCancel_ExitBack)
                CreateLoggerListMenu(param1);
        }
    }
}

void MenuHandler_SetAllLoggerLevel(Menu menu, MenuAction action, int param1, int param2)
{
    switch (action)
    {
        case MenuAction_Select:
        {
            char sLevel[32];
            menu.GetItem(param2, sLevel, sizeof(sLevel));
            Logger.ApplyAll(ApplyAllLogger_SetLevel, NameToLogLevel(sLevel));
            PrintToChat(param1, "[Log4sp] All loggers have been set to level %s.", sLevel);
        }

        case MenuAction_End:
            delete menu;

        case MenuAction_Cancel:
        {
            if (param2 == MenuCancel_ExitBack)
                CreateLoggerListMenu(param1);
        }
    }
}

void MenuHandler_SetAllLoggerFlushLevel(Menu menu, MenuAction action, int param1, int param2)
{
    switch (action)
    {
        case MenuAction_Select:
        {
            char sLevel[32];
            menu.GetItem(param2, sLevel, sizeof(sLevel));
            Logger.ApplyAll(ApplyAllLogger_SetFlushLevel, NameToLogLevel(sLevel));
            PrintToChat(param1, "[Log4sp] All loggers' flush level has been set to '%s.'", sLevel);
        }

        case MenuAction_End:
            delete menu;

        case MenuAction_Cancel:
        {
            if (param2 == MenuCancel_ExitBack)
                CreateLoggerListMenu(param1);
        }
    }
}

void ApplyAllLogger_SetLevel(Logger logger, LogLevel level)
{
    logger.SetLevel(level);
}

void ApplyAllLogger_SetFlushLevel(Logger logger, LogLevel level)
{
    logger.FlushOn(level);
}

void ApplyAllLogger_FlushAll(Logger logger)
{
    logger.Flush();
}

void MenuHandler_ManageLogger(Menu menu, MenuAction action, int param1, int param2)
{
    switch (action)
    {
        case MenuAction_Select:
        {
            char sLoggerName[64];
            menu.GetItem(0, sLoggerName, sizeof(sLoggerName));
            
            switch (param2)
            {
                case 3:
                {
                    Menu submenu = new Menu(MenuHandler_SetLoggerLevel);
                    submenu.SetTitle("Set log level for %s", sLoggerName);
                    submenu.AddItem(sLoggerName, "", ITEMDRAW_IGNORE);
                    submenu.AddItem(LOG4SP_LEVEL_NAME_OFF, "off");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_FATAL, "fatal");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_ERROR, "error");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_WARN, "warn");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_INFO, "info");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_DEBUG, "debug");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_TRACE, "trace");
                    submenu.ExitBackButton = true;
                    submenu.Display(param1, MENU_TIME_FOREVER);
                }

                case 4:
                {
                    Logger logger = Logger.Get(sLoggerName);
                    logger.Flush();
                    PrintToChat(param1, "[Log4sp] Logger '%s' flushed.", sLoggerName);
                }

                case 5:
                {
                    Menu submenu = new Menu(MenuHandler_SetLoggerFlushLevel);
                    submenu.SetTitle("Set logger flush level for %s", sLoggerName);
                    submenu.AddItem(sLoggerName, "", ITEMDRAW_IGNORE);
                    submenu.AddItem(LOG4SP_LEVEL_NAME_OFF, "off");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_FATAL, "fatal");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_ERROR, "error");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_WARN, "warn");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_INFO, "info");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_DEBUG, "debug");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_TRACE, "trace");
                    submenu.ExitBackButton = true;
                    submenu.Display(param1, MENU_TIME_FOREVER);
                }
            }
        }

        case MenuAction_End:
            delete menu;

        case MenuAction_Cancel:
        {
            if (param2 == MenuCancel_ExitBack)
                CreateLoggerListMenu(param1);
        }
    }
}

void MenuHandler_SetLoggerLevel(Menu menu, MenuAction action, int param1, int param2)
{
    switch (action)
    {
        case MenuAction_Select:
        {
            char sLoggerName[64];
            menu.GetItem(0, sLoggerName, sizeof(sLoggerName));
            Logger logger = Logger.Get(sLoggerName);

            char sName[32];
            menu.GetItem(param2, sName, sizeof(sName));

            logger.SetLevel(NameToLogLevel(sName));
            PrintToChat(param1, "[Log4sp] Logger '%s' level has been set to '%s'.", sLoggerName, sName);
        }

        case MenuAction_End:
            delete menu;

        case MenuAction_Cancel:
        {
            if (param2 == MenuCancel_ExitBack)
                CreateLoggerListMenu(param1);
        }
    }
}

void MenuHandler_SetLoggerFlushLevel(Menu menu, MenuAction action, int param1, int param2)
{
    switch (action)
    {
        case MenuAction_Select:
        {
            char sLoggerName[64];
            menu.GetItem(0, sLoggerName, sizeof(sLoggerName));
            Logger logger = Logger.Get(sLoggerName);

            char sName[32];
            menu.GetItem(param2, sName, sizeof(sName));

            logger.FlushOn(NameToLogLevel(sName));
            ReplyToCommand(param1, "[Log4sp] Logger '%s' flush level has been set to '%s'.", sLoggerName, sName);
        }

        case MenuAction_End:
            delete menu;

        case MenuAction_Cancel:
        {
            if (param2 == MenuCancel_ExitBack)
                CreateLoggerListMenu(param1);
        }
    }
}
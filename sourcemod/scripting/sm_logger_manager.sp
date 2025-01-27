#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <log4sp>

#define PLUGIN_VERSION LOG4SP_EXT_VERSION

public Plugin myinfo =
{
	name = "[Any] Logger Manager",
	author = "blueblur",
	description = "Instance manager for log4sp.",
	version = PLUGIN_VERSION,
	url = "https://github.com/F1F88/sm-ext-log4sp"
}

public void OnPluginStart()
{
    CreateConVar("sm_logger_manager_version", PLUGIN_VERSION, "The version of the Logger Manager plugin.", FCVAR_NOTIFY | FCVAR_DONTRECORD);

    RegAdminCmd("sm_log4sp", Cmd_Logger, ADMFLAG_CONFIG, "Open logger menu.");
    RegAdminCmd("sm_log4sp_applyall", Cmd_Logger_ApplyAll, ADMFLAG_CONFIG, "Apply logger settings to all instances.");
    RegAdminCmd("sm_log4sp_setlevel", Cmd_Logger_SetLevel, ADMFLAG_CONFIG, "Set log level for a logger instance.");
    RegAdminCmd("sm_log4sp_setpattern", Cmd_Logger_SetPattern, ADMFLAG_CONFIG, "Set log pattern for a logger instance.");
    RegAdminCmd("sm_log4sp_log", Cmd_Logger_Log, ADMFLAG_CONFIG, "Log a message to a logger instance.");
    RegAdminCmd("sm_log4sp_flush", Cmd_Logger_Flush, ADMFLAG_CONFIG, "Flush a logger instance.");
    RegAdminCmd("sm_log4sp_setflushlevel", Cmd_Logger_SetFlushLevel, ADMFLAG_CONFIG, "Close a logger instance.");
}

Action Cmd_Logger(int client, int args)
{
    if (!client)
    {
        ReplyToCommand(client, "[SM] This command can only be used in-game.");
        return Plugin_Handled;
    }

    CreateLoggerListMenu(client);
    return Plugin_Handled;
}

void CreateLoggerListMenu(int client)
{
    ArrayList hArray = new ArrayList(ByteCountToCells(64));
    Logger.ApplyAll(ApplyAllLogger_GetAllLoggerName, hArray);

    if (!hArray.Length)
    {
        ReplyToCommand(client, "[SM] No loggers found.");
        delete hArray;
        return;
    }

    Menu menu = new Menu(MenuHandler_LoggerList);
    menu.SetTitle("Select a logger to manager:");

    for (int i = 0; i < hArray.Length; i++)
    {
        char sName[64];
        hArray.GetString(i, sName, sizeof(sName));
        menu.AddItem(sName, sName);
    }

    delete hArray;
    menu.AddItem("X", "Apply to all loggers");
    menu.Display(client, MENU_TIME_FOREVER);
}

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
            if (param2 == menu.ItemCount - 1)
            {
                Menu submenu = new Menu(MenuHandler_ApplyToAll);
                submenu.SetTitle("Choose an action to apply to all loggers:");
                submenu.AddItem("1", "Set All logger's level");
                submenu.AddItem("2", "Flush loggers");
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
                    ReplyToCommand(param1, "[SM] Logger %s not found.", sInfo);
                    return;
                }

                Menu submenu = new Menu(MenuHandler_ManageLogger);
                submenu.SetTitle("Manage Logger: %s", sInfo);
                submenu.AddItem(sInfo, sInfo, ITEMDRAW_IGNORE);
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
                    submenu.AddItem(LOG4SP_LEVEL_NAME_OFF, "Off");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_FATAL, "Fatal");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_ERROR, "Error");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_WARN, "Warn");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_INFO, "Info");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_DEBUG, "Debug");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_TRACE, "Trace");
                    submenu.ExitBackButton = true;
                    submenu.Display(param1, MENU_TIME_FOREVER);
                }

                case 1:
                {
                    Logger.ApplyAll(ApplyAllLogger_FlushAll);
                    ReplyToCommand(param1, "[SM] All loggers have been flushed.");
                }

                case 2:
                {
                    Menu submenu = new Menu(MenuHandler_SetAllLoggerFlushLevel);
                    submenu.SetTitle("Set flush level for all loggers:");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_OFF, "Off");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_FATAL, "Fatal");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_ERROR, "Error");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_WARN, "Warn");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_INFO, "Info");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_DEBUG, "Debug");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_TRACE, "Trace");
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
            LogLevel lvl = NameToLogLevel(sLevel);
            Logger.ApplyAll(ApplyAllLogger_SetLevel, lvl);
            ReplyToCommand(param1, "[SM] All loggers have set to level %s.", sLevel);
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
            LogLevel lvl = NameToLogLevel(sLevel);
            Logger.ApplyAll(ApplyAllLogger_SetFlushLevel, lvl);
            ReplyToCommand(param1, "[SM] All loggers have set to flush level %s.", sLevel);
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
                case 1:
                {
                    Menu submenu = new Menu(MenuHandler_SetLoggerLevel);
                    submenu.SetTitle("Set log level for %s", sLoggerName);
                    submenu.AddItem(sLoggerName, sLoggerName, ITEMDRAW_IGNORE);
                    submenu.AddItem(LOG4SP_LEVEL_NAME_OFF, "Off");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_FATAL, "Fatal");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_ERROR, "Error");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_WARN, "Warn");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_INFO, "Info");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_DEBUG, "Debug");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_TRACE, "Trace");
                    submenu.ExitBackButton = true;
                    submenu.Display(param1, MENU_TIME_FOREVER);
                }

                case 2:
                {
                    Logger logger = Logger.Get(sLoggerName);
                    logger.Flush();
                    ReplyToCommand(param1, "[SM] Logger %s flushed.", sLoggerName);
                }

                case 3:
                {
                    Menu submenu = new Menu(MenuHandler_SetLoggerFlushLevel);
                    submenu.SetTitle("Set logger auto flush level for %s", sLoggerName);
                    submenu.AddItem(sLoggerName, sLoggerName, ITEMDRAW_IGNORE);
                    submenu.AddItem(LOG4SP_LEVEL_NAME_OFF, "Off");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_FATAL, "Fatal");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_ERROR, "Error");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_WARN, "Warn");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_INFO, "Info");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_DEBUG, "Debug");
                    submenu.AddItem(LOG4SP_LEVEL_NAME_TRACE, "Trace");
                    submenu.ExitBackButton = true;
                    submenu.Display(param1, MENU_TIME_FOREVER);
                }

                case 4:
                {
                    Logger logger = Logger.Get(sLoggerName);
                    if (!logger)
                    {
                        ReplyToCommand(param1, "[SM] Logger %s not found or already deleted.", sLoggerName);
                        return;
                    }

                    delete logger;
                    ReplyToCommand(param1, "[SM] Logger %s deleted.", sLoggerName);
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

            LogLevel lvl = NameToLogLevel(sName);
            logger.SetLevel(lvl);

            ReplyToCommand(param1, "[SM] Logger %s level set to %s.", sLoggerName, sName);
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

            LogLevel lvl = NameToLogLevel(sName);
            logger.FlushOn(lvl);
            ReplyToCommand(param1, "[SM] Logger %s auto flush level set to %s.", sLoggerName, sName);
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

// sm_log4sp_applyall <function_name> [arguments]
Action Cmd_Logger_ApplyAll(int client, int args)
{
    if (!client)
    {
        ReplyToCommand(client, "[SM] This command can only be used in-game.");
        return Plugin_Handled;
    }

    int iArgc = GetCmdArgs();
    if (iArgc < 2 || iArgc > 3)
    {
        ReplyToCommand(client, "[SM] Usage: sm_log4sp_applyall [function name] [arguments] [<optional> log level]");
        return Plugin_Handled;
    }

    char sFunctionName[16];
    GetCmdArg(1, sFunctionName, sizeof(sFunctionName));

    char sArg[32];
    GetCmdArg(2, sArg, sizeof(sArg));

    DataPack dp = new DataPack();
    dp.Reset();
    dp.WriteString(sFunctionName);
    dp.WriteString(sArg);

    bool bHasLevel = false;
    if (!strcmp(sFunctionName, "log"))
    {
        if (iArgc == 3)
        {
            char sLvl[16];
            GetCmdArg(3, sLvl, sizeof(sLvl));
            LogLevel lvl = NameToLogLevel(sLvl);
            bHasLevel = true;
            dp.WriteCell(bHasLevel);
            dp.WriteCell(lvl);
        }
        else
        {
            dp.WriteCell(bHasLevel);
        }
    }

    Logger.ApplyAll(ApplyAllLogger_Excute, dp);
    ReplyToCommand(client, "[SM] All loggers have been applied with command: '%s %s'.", sFunctionName, sArg);
    return Plugin_Handled;
}

void ApplyAllLogger_Excute(Logger logger, DataPack dp)
{
    static char sArg[32];
    static char sFunctionName[16];
    LogLevel lvl;

    if (dp)
    {
        dp.Reset();
        dp.ReadString(sFunctionName, sizeof(sFunctionName));
        dp.ReadString(sArg, sizeof(sArg));
        if (dp.IsReadable())
        {
            bool bHasLevel = view_as<bool>(dp.ReadCell());
            if (bHasLevel) lvl = view_as<LogLevel>(dp.ReadCell());
        }

        delete dp;
    }

    if (!strcmp(sFunctionName, "set_lvl", false))
    {
        lvl = NameToLogLevel(sArg);
        logger.SetLevel(lvl);
    }
    else if (!strcmp(sFunctionName, "set_pattern"))
    {
        logger.SetPattern(sArg);
    }
    else if (!strcmp(sFunctionName, "log"))
    {
        logger.Log(lvl, sArg);
    }
    else if (!strcmp(sFunctionName, "flush"))
    {
        logger.Flush();
    }
    else if (!strcmp(sFunctionName, "set_flush_lvl"))
    {
        lvl = NameToLogLevel(sArg);
        logger.FlushOn(lvl);
    }
}

Action Cmd_Logger_SetLevel(int client, int args)
{
    if (!client)
    {
        ReplyToCommand(client, "[SM] This command can only be used in-game.");
        return Plugin_Handled;
    }

    if (GetCmdArgs() != 2)
    {
        ReplyToCommand(client, "[SM] Usage: sm_log4sp_setlevel [logger name] [level]");
        return Plugin_Handled;
    }

    char sName[64], sArg[16];
    GetCmdArg(1, sName, sizeof(sName));
    Logger logger = Logger.Get(sName);

    if (!logger) 
    {
        ReplyToCommand(client, "[SM] Logger '%s' not found.", sName);
        return Plugin_Handled;
    }

    GetCmdArg(2, sArg, sizeof(sArg));
    LogLevel lvl = NameToLogLevel(sArg);
    logger.SetLevel(lvl);

    return Plugin_Handled;
}

Action Cmd_Logger_SetPattern(int client, int args)
{
    if (!client)
    {
        ReplyToCommand(client, "[SM] This command can only be used in-game.");
        return Plugin_Handled;
    }

    if (GetCmdArgs() != 2)
    {
        ReplyToCommand(client, "[SM] Usage: sm_log4sp_setpattern [logger name] [pattern]");
        return Plugin_Handled;
    }

    char sName[64], sArg[1024];
    GetCmdArg(1, sName, sizeof(sName));
    Logger logger = Logger.Get(sName);

    if (!logger) 
    {
        ReplyToCommand(client, "[SM] Logger '%s' not found.", sName);
        return Plugin_Handled;
    }

    GetCmdArg(2, sArg, sizeof(sArg));
    logger.SetPattern(sArg);
    return Plugin_Handled;
}

Action Cmd_Logger_Log(int client, int args)
{
    if (!client)
    {
        ReplyToCommand(client, "[SM] This command can only be used in-game.");
        return Plugin_Handled;
    }

    int iArgc = GetCmdArgs();
    if (iArgc < 2 || iArgc > 3)
    {
        ReplyToCommand(client, "[SM] Usage: sm_log4sp_log [logger name] [message] [<optional> log level]");
        return Plugin_Handled;
    }

    char sName[64];
    GetCmdArg(1, sName, sizeof(sName));
    Logger logger = Logger.Get(sName);

    if (!logger) 
    {
        ReplyToCommand(client, "[SM] Logger '%s' not found.", sName);
        return Plugin_Handled;
    }
    
    LogLevel lvl;
    if (iArgc == 3)
    {
        char sLevel[32];
        GetCmdArg(3, sLevel, sizeof(sLevel));
        lvl = NameToLogLevel(sLevel);
    }
    else if (iArgc == 2)
    {
        lvl = logger.GetLevel();
    }

    char sArg[1024];
    GetCmdArg(2, sArg, sizeof(sArg));
    logger.Log(lvl, sArg);
    return Plugin_Handled;
}

Action Cmd_Logger_Flush(int client, int args)
{
    if (!client)
    {
        ReplyToCommand(client, "[SM] This command can only be used in-game.");
        return Plugin_Handled;
    }

    if (GetCmdArgs() != 1)
    {
        ReplyToCommand(client, "[SM] Usage: sm_log4sp_flush [logger name]");
        return Plugin_Handled;
    }

    char sName[64];
    GetCmdArg(1, sName, sizeof(sName));
    Logger logger = Logger.Get(sName);

    if (!logger) 
    {
        ReplyToCommand(client, "[SM] Logger '%s' not found.", sName);
        return Plugin_Handled;
    }

    logger.Flush();
    return Plugin_Handled;
}

Action Cmd_Logger_SetFlushLevel(int client, int args)
{
    if (!client)
    {
        ReplyToCommand(client, "[SM] This command can only be used in-game.");
        return Plugin_Handled;
    }

    if (GetCmdArgs() != 2)
    {
        ReplyToCommand(client, "[SM] Usage: sm_log4sp_setflushlevel [logger name] [level]");
        return Plugin_Handled;
    }

    char sName[64], sArg[16];
    GetCmdArg(1, sName, sizeof(sName));
    Logger logger = Logger.Get(sName);

    if (!logger) 
    {
        ReplyToCommand(client, "[SM] Logger '%s' not found.", sName);
        return Plugin_Handled;
    }

    GetCmdArg(2, sArg, sizeof(sArg));
    LogLevel lvl = NameToLogLevel(sArg);
    logger.FlushOn(lvl);

    return Plugin_Handled;
}
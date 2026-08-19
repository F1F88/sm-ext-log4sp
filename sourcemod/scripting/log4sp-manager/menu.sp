#pragma semicolon 1
#pragma newdecls required

#include <log4sp/common>

#undef REQUIRE_PLUGIN
#include <log4sp/registry>
#define REQUIRE_PLUGIN


void RegisterMenuCommands()
{
    RegAdminCmd("sm_log4sp_menu", Cmd_Log4spMenu, ADMFLAG_CONFIG, "Displays the log4sp manager menu");
    RegAdminCmd("sm_log4sp_manager", Cmd_Log4spMenu, ADMFLAG_CONFIG, "Displays the log4sp manager menu");
    RegAdminCmd("sm_log4sp_manager_menu", Cmd_Log4spMenu, ADMFLAG_CONFIG, "Displays the log4sp manager menu");
}


static Action Cmd_Log4spMenu(int client, int args)
{
    if (!IsValidClient(client))
    {
        ReplyToCommand(client, "[SM] %t", "Command is in-game only");
        return Plugin_Handled;
    }

    DisplayManagerMenu(client);

    return Plugin_Handled;
}


static void DisplayManagerMenu(int client, int time = MENU_TIME_FOREVER)
{
    if (!IsValidClient(client))
        return;

    ArrayList names = new ArrayList(ByteCountToCells(64));
    Registry.Instance().ApplyAll(null, ApplyAllLogger_GetNames, names);

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

static void DisplayApplyAllMenu(int client, int time = MENU_TIME_FOREVER)
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

static void DisplayApplyAllSetLogLvlMenu(int client, int time = MENU_TIME_FOREVER)
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

static void DisplayApplyAllSetFlushLvlMenu(int client, int time = MENU_TIME_FOREVER)
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

static void DisplayLoggerMenu(const char[] name, int client, int time = MENU_TIME_FOREVER)
{
    if (!IsValidClient(client))
        return;

    Logger logger = Registry.Instance().Get(name);
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

static void DisplayLoggerSetLogLvlMenu(const char[] name, int client, int time = MENU_TIME_FOREVER)
{
    if (!IsValidClient(client))
        return;

    Logger logger = Registry.Instance().Get(name);
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

static void DisplayLoggerSetFlushLvlMenu(const char[] name, int client, int time = MENU_TIME_FOREVER)
{
    if (!IsValidClient(client))
        return;

    Logger logger = Registry.Instance().Get(name);
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


static int MenuHandler_Manager(Menu menu, MenuAction action, int param1, int param2)
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
                if (!Registry.Instance().Get(name))
                {
                    PrintToChat(param1, "[SM] Logger with \"%s\" not exists.", name);
                    return 0;
                }

                DisplayLoggerMenu(name, param1);
            }
        }
        case MenuAction_End:
        {
            delete menu;
        }
    }
    return 0;
}

static int MenuHandler_ApplyAll(Menu menu, MenuAction action, int param1, int param2)
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
                    Registry.Instance().ApplyAll(null, ApplyAllLogger_FlushAll);
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
    return 0;
}

static int MenuHandler_AllSetLogLvl(Menu menu, MenuAction action, int param1, int param2)
{
    switch (action)
    {
        case MenuAction_Select:
        {
            char level[32];
            menu.GetItem(param2, level, sizeof(level));
            PrintToChat(param1, "[SM] All loggers will set log level to '%s'.", level);
            Registry.Instance().ApplyAll(null, ApplyAllLogger_SetLevel, NameToLogLevel(level));
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
    return 0;
}

static int MenuHandler_AllSetFlushLvl(Menu menu, MenuAction action, int param1, int param2)
{
    switch (action)
    {
        case MenuAction_Select:
        {
            char level[32];
            menu.GetItem(param2, level, sizeof(level));
            PrintToChat(param1, "[SM] All loggers will set flush level to '%s'.", level);
            Registry.Instance().ApplyAll(null, ApplyAllLogger_SetFlushLevel, NameToLogLevel(level));
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
    return 0;
}

static int MenuHandler_Logger(Menu menu, MenuAction action, int param1, int param2)
{
    switch (action)
    {
        case MenuAction_Select:
        {
            const int nameIndex = 3;
            char name[64];
            menu.GetItem(nameIndex, name, sizeof(name));
            Logger logger = Registry.Instance().Get(name);
            if (!logger)
            {
                PrintToChat(param1, "[SM] Logger with \"%s\" not exists.", name);
                return 0;
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
    return 0;
}

static int MenuHandler_SetLogLvl(Menu menu, MenuAction action, int param1, int param2)
{
    switch (action)
    {
        case MenuAction_Select:
        {
            const int nameIndex = view_as<int>(LogLevel_Total);
            char name[64];
            menu.GetItem(nameIndex, name, sizeof(name));
            Logger logger = Registry.Instance().Get(name);
            if (!logger)
            {
                PrintToChat(param1, "[SM] Logger with \"%s\" not exists.", name);
                return 0;
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
    return 0;
}

static int MenuHandler_SetFlushLvl(Menu menu, MenuAction action, int param1, int param2)
{
    switch (action)
    {
        case MenuAction_Select:
        {
            const int nameIndex = view_as<int>(LogLevel_Total);
            char name[64];
            menu.GetItem(nameIndex, name, sizeof(name));
            Logger logger = Registry.Instance().Get(name);
            if (!logger)
            {
                PrintToChat(param1, "[SM] Logger with \"%s\" not exists.", name);
                return 0;
            }

            char level[32];
            menu.GetItem(param2, level, sizeof(level));

            PrintToChat(param1, "[SM] Logger '%s' will set flush level to '%s'.", name, level);
            logger.SetFlushLevel(NameToLogLevel(level));
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
    return 0;
}


static void ApplyAllLogger_GetNames(Logger logger, ArrayList array)
{
    char buffer[64];
    logger.GetName(buffer, sizeof(buffer));
    array.PushString(buffer);
}

static void ApplyAllLogger_FlushAll(Logger logger)
{
    logger.Flush();
}

static void ApplyAllLogger_SetLevel(Logger logger, LogLevel level)
{
    logger.SetLevel(level);
}

static void ApplyAllLogger_SetFlushLevel(Logger logger, LogLevel level)
{
    logger.SetFlushLevel(level);
}

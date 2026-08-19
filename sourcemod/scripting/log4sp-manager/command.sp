#pragma semicolon 1
#pragma newdecls required

#include <regex>
#include <log4sp/common>

#undef REQUIRE_PLUGIN
#include <log4sp/registry>
#define REQUIRE_PLUGIN

#if !defined COMMAND_MAX_LENGTH
    #define  COMMAND_MAX_LENGTH 512     // https://github.com/alliedmodders/sourcemod/pull/1732
#endif

#define LOG4SP_LEVEL_NAMES  "["...LOG4SP_LEVEL_NAME_TRACE..." < "...LOG4SP_LEVEL_NAME_DEBUG..." < "...LOG4SP_LEVEL_NAME_INFO..." < "...LOG4SP_LEVEL_NAME_WARN..." < "...LOG4SP_LEVEL_NAME_ERROR..." < "...LOG4SP_LEVEL_NAME_FATAL..." < "...LOG4SP_LEVEL_NAME_OFF..."]"


enum CommandCode
{
    CmdCode_OK = 0,
    CmdCode_BadArgCount,
    CmdCode_InvalidArg                  // This is usually because the logger does not exist
}

enum
{
    Cmd_Log = 0,
    Cmd_ShouldLog,
    Cmd_GetLvl,
    Cmd_SetLvl,
    Cmd_SetPattern,
    Cmd_Flush,
    Cmd_ShouldFlush,
    Cmd_GetFlushLvl,
    Cmd_SetFlushLvl,
    Cmd_ApplyAll,
    Cmd_List,
    Cmd_Version,

    Cmd_Total
}


static const char COMMANDS[][] = {
    "log",
    "should_log",
    "get_lvl",
    "set_lvl",
    "set_pattern",
    "flush",
    "should_flush",
    "get_flush_lvl",
    "set_flush_lvl",
    "apply_all",
    "list",
    "version"
};

static const char COMMANDS_TEXT[][] = {
    "Use a logger to log a message.",
    "Gets a logger whether logging for the given log level.",
    "Gets a logger log level. " ... LOG4SP_LEVEL_NAMES,
    "Sets a logger log level. " ... LOG4SP_LEVEL_NAMES,
    "Sets a logger pattern.",
    "Manual flush a logger contents.",
    "Gets a logger whether trigger automatic flush for the given log level.",
    "Gets a logger flush level.",
    "Sets a logger flush level.",
    "Apply a command to all loggers.",
    "List all logger names.",
    "Display version information."
};

static const char COMMANDS_USAGE[][] = {
    "sm_log4sp log <logger_name> <level> [message]",
    "sm_log4sp should_log <logger_name> <level>",
    "sm_log4sp get_lvl <logger_name>",
    "sm_log4sp set_lvl <logger_name> <level>",
    "sm_log4sp set_pattern <logger_name> <pattern> [pattern_time_type]",
    "sm_log4sp flush <logger_name>",
    "sm_log4sp should_flush <logger_name> <level>",
    "sm_log4sp get_flush_lvl <logger_name>",
    "sm_log4sp set_flush_lvl <logger_name> <level>",
    "sm_log4sp apply_all <command> [arguments]",
    "sm_log4sp list",
    "sm_log4sp version"
};

static const int COMMANDS_REQUIRE_ARG[] = {3, 3, 2, 3, 3, 2, 3, 2, 3, 2, 1, 1};

static Function _fCommandsFunc[Cmd_Total];


void RegisterCommandCommands()
{
    RegAdminCmd("sm_log4sp", Cmd_Log4sp, ADMFLAG_CONFIG, "Log4sp console command menu.");

    // Initialize command function table
    _fCommandsFunc[Cmd_Log]         = CmdExecute_Log;
    _fCommandsFunc[Cmd_ShouldLog]   = CmdExecute_ShouldLog;
    _fCommandsFunc[Cmd_GetLvl]      = CmdExecute_GetLvl;
    _fCommandsFunc[Cmd_SetLvl]      = CmdExecute_SetLvl;
    _fCommandsFunc[Cmd_SetPattern]  = CmdExecute_SetPattern;
    _fCommandsFunc[Cmd_Flush]       = CmdExecute_Flush;
    _fCommandsFunc[Cmd_ShouldFlush] = CmdExecute_ShouldFlush;
    _fCommandsFunc[Cmd_GetFlushLvl] = CmdExecute_GetFlushLvl;
    _fCommandsFunc[Cmd_SetFlushLvl] = CmdExecute_SetFlushLvl;
    _fCommandsFunc[Cmd_ApplyAll]    = CmdExecute_ApplyAll;
    _fCommandsFunc[Cmd_List]        = CmdExecute_List;
    _fCommandsFunc[Cmd_Version]     = CmdExecute_Version;
}


static Action Cmd_Log4sp(int client, int args)
{
    char command[COMMAND_MAX_LENGTH];
    GetCmdArg(1, command, sizeof(command));

    // HACK: 还有更好的办法吗? 我也不知道为什么要用这种邪修方案, 反正就是用了
    // Find the target function according to the command
    for (int i = 0; i < Cmd_Total; ++i)
    {
        if (!StrEqual(command, COMMANDS[i], false))
            continue;

        // Check if the args meets the function requirements
        if (args < COMMANDS_REQUIRE_ARG[i])
        {
            PrintToServer("[SM] Usage: %s", COMMANDS_USAGE[i]);
            return Plugin_Handled;
        }

        // Packaging command arguments
        char buffer[COMMAND_MAX_LENGTH];
        DataPack data = new DataPack();
        for (int j = 2; j <= args; ++j)
        {
            GetCmdArg(j, buffer, sizeof(buffer));
            data.WriteString(buffer);
        }

        // Call the target function to process
        Call_StartFunction(null, _fCommandsFunc[i]);
        Call_PushCell(data);
        Call_Finish();
        delete data;
        return Plugin_Handled;
    }

    // Command is incorrect
    // Draw a menu for re-entry
    CmdExecute_DrawMenu();
    return Plugin_Handled;
}


static CommandCode CmdExecute_DrawMenu()
{
    PrintToServer("Log4sp Menu:");
    PrintToServer("Usage: sm_log4sp <command> [arguments]");

    // 照葫芦画瓢
    // https://github.com/alliedmodders/sourcemod/blob/1bc536fdfa0fe088300ae3532ede2da84b7a80d9/core/logic/RootConsoleMenu.cpp#L156
    for (int i = 0; i < sizeof(COMMANDS); ++i)
        PrintToServer("    %-16.16s - %s", COMMANDS[i], COMMANDS_TEXT[i]);

    return CmdCode_OK;
}


static CommandCode CmdExecute_Log(DataPack data)
{
    data.Reset();

    if (!data.IsReadable())
    {
        PrintToServer("[SM] Usage: %s", COMMANDS_USAGE[Cmd_Log]);
        return CmdCode_BadArgCount;
    }
    char name[COMMAND_MAX_LENGTH];
    data.ReadString(name, sizeof(name));

    Logger logger = Registry.Instance().Get(name);
    if (!logger)
    {
        PrintToServer("[SM] Logger \"%s\" not exists.", name);
        return CmdCode_InvalidArg;
    }

    if (!data.IsReadable())
    {
        PrintToServer("[SM] Usage: %s", COMMANDS_USAGE[Cmd_Log]);
        return CmdCode_BadArgCount;
    }
    char level[8];
    data.ReadString(level, sizeof(level));

    LogLevel lvl = NameToLogLevel(level);
    if (lvl == LogLevel_Off)
        StringToIntEx(level, view_as<int>(lvl));
    LogLevelToName(level, sizeof(level), lvl);

    char msg[COMMAND_MAX_LENGTH];
    if (data.IsReadable())
        data.ReadString(msg, sizeof(msg));

    logger.Log(lvl, msg);
    PrintToServer("[SM] Logger \"%s\" log a \"%s\" level message \"%s\".", name, level, msg);
    return CmdCode_OK;
}

static CommandCode CmdExecute_ShouldLog(DataPack data)
{
    data.Reset();

    if (!data.IsReadable())
    {
        PrintToServer("[SM] Usage: %s", COMMANDS_USAGE[Cmd_ShouldLog]);
        return CmdCode_BadArgCount;
    }
    char name[COMMAND_MAX_LENGTH];
    data.ReadString(name, sizeof(name));

    Logger logger = Registry.Instance().Get(name);
    if (!logger)
    {
        PrintToServer("[SM] Logger \"%s\" not exists.", name);
        return CmdCode_InvalidArg;
    }

    if (!data.IsReadable())
    {
        PrintToServer("[SM] Usage: %s", COMMANDS_USAGE[Cmd_ShouldLog]);
        return CmdCode_BadArgCount;
    }
    char level[8];
    data.ReadString(level, sizeof(level));

    LogLevel lvl = NameToLogLevel(level);
    if (lvl == LogLevel_Off)
        StringToIntEx(level, view_as<int>(lvl));
    LogLevelToName(level, sizeof(level), lvl);

    if (logger.ShouldLog(lvl))
        PrintToServer("[SM] Logger \"%s\" are enabled logging for \"%s\" level.", name, level);
    else
        PrintToServer("[SM] Logger \"%s\" are not enabled logging for \"%s\" level.", name, level);
    return CmdCode_OK;
}

static CommandCode CmdExecute_GetLvl(DataPack data)
{
    data.Reset();

    if (!data.IsReadable())
    {
        PrintToServer("[SM] Usage: %s", COMMANDS_USAGE[Cmd_GetLvl]);
        return CmdCode_BadArgCount;
    }
    char name[COMMAND_MAX_LENGTH];
    data.ReadString(name, sizeof(name));

    Logger logger = Registry.Instance().Get(name);
    if (!logger)
    {
        PrintToServer("[SM] Logger \"%s\" not exists.", name);
        return CmdCode_InvalidArg;
    }

    char level[8];
    LogLevelToName(level, sizeof(level), logger.GetLevel());

    PrintToServer("[SM] Logger \"%s\" log level is \"%s\".", name, level);
    return CmdCode_OK;
}

static CommandCode CmdExecute_SetLvl(DataPack data)
{
    data.Reset();

    if (!data.IsReadable())
    {
        PrintToServer("[SM] Usage: %s", COMMANDS_USAGE[Cmd_SetLvl]);
        return CmdCode_BadArgCount;
    }
    char name[COMMAND_MAX_LENGTH];
    data.ReadString(name, sizeof(name));

    Logger logger = Registry.Instance().Get(name);
    if (!logger)
    {
        PrintToServer("[SM] Logger \"%s\" not exists.", name);
        return CmdCode_InvalidArg;
    }

    if (!data.IsReadable())
    {
        PrintToServer("[SM] Usage: %s", COMMANDS_USAGE[Cmd_SetLvl]);
        return CmdCode_BadArgCount;
    }
    char level[8];
    data.ReadString(level, sizeof(level));

    LogLevel lvl = NameToLogLevel(level);
    if (lvl == LogLevel_Off)
        StringToIntEx(level, view_as<int>(lvl));
    LogLevelToName(level, sizeof(level), lvl);

    char oldLevel[8];
    LogLevelToName(oldLevel, sizeof(oldLevel), logger.GetLevel());

    logger.SetLevel(lvl);
    PrintToServer("[SM] Logger \"%s\" set log level to \"%s\". (original: \"%s\")", name, level, oldLevel);
    return CmdCode_OK;
}

static CommandCode CmdExecute_SetPattern(DataPack data)
{
    data.Reset();

    if (!data.IsReadable())
    {
        PrintToServer("[SM] Usage: %s", COMMANDS_USAGE[Cmd_SetPattern]);
        return CmdCode_BadArgCount;
    }
    char name[COMMAND_MAX_LENGTH];
    data.ReadString(name, sizeof(name));

    Logger logger = Registry.Instance().Get(name);
    if (!logger)
    {
        PrintToServer("[SM] Logger \"%s\" not exists.", name);
        return CmdCode_InvalidArg;
    }

    if (!data.IsReadable())
    {
        PrintToServer("[SM] Usage: %s", COMMANDS_USAGE[Cmd_SetPattern]);
        return CmdCode_BadArgCount;
    }
    char pattern[COMMAND_MAX_LENGTH];
    data.ReadString(pattern, sizeof(pattern));

    char type[COMMAND_MAX_LENGTH];
    if (data.IsReadable())
        data.ReadString(type, sizeof(type));

    logger.SetPattern(pattern, view_as<PatternTimeType>(StringToInt(type)));
    PrintToServer("[SM] Logger \"%s\" set pattern to \"%s\".", name, pattern);
    return CmdCode_OK;
}

static CommandCode CmdExecute_Flush(DataPack data)
{
    data.Reset();

    if (!data.IsReadable())
    {
        PrintToServer("[SM] Usage: %s", COMMANDS_USAGE[Cmd_Flush]);
        return CmdCode_BadArgCount;
    }
    char name[COMMAND_MAX_LENGTH];
    data.ReadString(name, sizeof(name));

    Logger logger = Registry.Instance().Get(name);
    if (!logger)
    {
        PrintToServer("[SM] Logger \"%s\" not exists.", name);
        return CmdCode_InvalidArg;
    }

    logger.Flush();
    PrintToServer("[SM] Logger \"%s\" flush its contents.", name);
    return CmdCode_OK;
}

static CommandCode CmdExecute_ShouldFlush(DataPack data)
{
    data.Reset();

    if (!data.IsReadable())
    {
        PrintToServer("[SM] Usage: %s", COMMANDS_USAGE[Cmd_ShouldFlush]);
        return CmdCode_BadArgCount;
    }
    char name[COMMAND_MAX_LENGTH];
    data.ReadString(name, sizeof(name));

    Logger logger = Registry.Instance().Get(name);
    if (!logger)
    {
        PrintToServer("[SM] Logger \"%s\" not exists.", name);
        return CmdCode_InvalidArg;
    }

    if (!data.IsReadable())
    {
        PrintToServer("[SM] Usage: %s", COMMANDS_USAGE[Cmd_ShouldFlush]);
        return CmdCode_BadArgCount;
    }
    char level[8];
    data.ReadString(level, sizeof(level));

    LogLevel lvl = NameToLogLevel(level);
    if (lvl == LogLevel_Off)
        StringToIntEx(level, view_as<int>(lvl));
    LogLevelToName(level, sizeof(level), lvl);

    if (logger.ShouldFlush(lvl))
        PrintToServer("[SM] Logger \"%s\" are trigger automatic flush for \"%s\" level.", name, level);
    else
        PrintToServer("[SM] Logger \"%s\" are not trigger automatic flush for \"%s\" level.", name, level);
    return CmdCode_OK;
}

static CommandCode CmdExecute_GetFlushLvl(DataPack data)
{
    data.Reset();

    if (!data.IsReadable())
    {
        PrintToServer("[SM] Usage: %s", COMMANDS_USAGE[Cmd_GetFlushLvl]);
        return CmdCode_BadArgCount;
    }
    char name[COMMAND_MAX_LENGTH];
    data.ReadString(name, sizeof(name));

    Logger logger = Registry.Instance().Get(name);
    if (!logger)
    {
        PrintToServer("[SM] Logger \"%s\" not exists.", name);
        return CmdCode_InvalidArg;
    }

    char level[8];
    LogLevelToName(level, sizeof(level), logger.GetFlushLevel());

    PrintToServer("[SM] Logger \"%s\" flush level is \"%s\".", name, level);
    return CmdCode_OK;
}

static CommandCode CmdExecute_SetFlushLvl(DataPack data)
{
    data.Reset();

    if (!data.IsReadable())
    {
        PrintToServer("[SM] Usage: %s", COMMANDS_USAGE[Cmd_SetFlushLvl]);
        return CmdCode_BadArgCount;
    }
    char name[COMMAND_MAX_LENGTH];
    data.ReadString(name, sizeof(name));

    Logger logger = Registry.Instance().Get(name);
    if (!logger)
    {
        PrintToServer("[SM] Logger \"%s\" not exists.", name);
        return CmdCode_InvalidArg;
    }


    if (!data.IsReadable())
    {
        PrintToServer("[SM] Usage: %s", COMMANDS_USAGE[Cmd_SetFlushLvl]);
        return CmdCode_BadArgCount;
    }
    char level[8];
    data.ReadString(level, sizeof(level));

    LogLevel lvl = NameToLogLevel(level);
    if (lvl == LogLevel_Off)
        StringToIntEx(level, view_as<int>(lvl));
    LogLevelToName(level, sizeof(level), lvl);

    char oldLevel[8];
    LogLevelToName(oldLevel, sizeof(oldLevel), logger.GetFlushLevel());

    logger.SetFlushLevel(lvl);
    PrintToServer("[SM] Logger \"%s\" set flush level to \"%s\". (original: \"%s\")", name, level, oldLevel);
    return CmdCode_OK;
}

static CommandCode CmdExecute_ApplyAll(DataPack data)
{
    data.Reset();

    if (!data.IsReadable())
    {
        PrintToServer("[SM] Usage: %s", COMMANDS_USAGE[Cmd_ApplyAll]);
        return CmdCode_BadArgCount;
    }
    char command[COMMAND_MAX_LENGTH];
    data.ReadString(command, sizeof(command));

    for (int i = 0; i <= Cmd_SetFlushLvl; ++i)
    {
        if (!StrEqual(command, COMMANDS[i], false))
            continue;

        // HACK: The only difference is that `apply_all` replaces `logger_name`,
        // so the number of required arguments is the same.
        if (GetCmdArgs() < COMMANDS_REQUIRE_ARG[i])
        {
            // HACK: Extract the arguments after <logger_name>
            // The IF check ensures that there are arguments,
            // therefore the index will not go out of bounds.
            int index = FindCharInString(COMMANDS_USAGE[i], '>') + 1;
            PrintToServer("[SM] Usage: sm_log4sp apply_all %s%s", command, COMMANDS_USAGE[i][index]);
            return CmdCode_BadArgCount;
        }

        // Get all registered logger names
        StringMap loggers = new StringMap();
        Registry.Instance().ApplyAll(null, CB_OnApplyAll_GetNames, loggers);
        StringMapSnapshot snapshot = loggers.Snapshot();
        delete loggers;

        // Execute commands for each logger
        for (int j = 0; j < snapshot.Length; ++j)
        {
            // Logger name to be executed
            char buffer[COMMAND_MAX_LENGTH];
            snapshot.GetKey(j, buffer, sizeof(buffer));

            DataPack funcData = new DataPack();
            funcData.WriteString(buffer);

            // Push command parameters
            data.Reset();
            data.ReadString("", 0);     // skip apply_all
            while (data.IsReadable())
            {
                data.ReadString(buffer, sizeof(buffer));
                funcData.WriteString(buffer);
            }

            Call_StartFunction(null, _fCommandsFunc[i]);
            Call_PushCell(funcData);
            Call_Finish();
            delete funcData;
        }

        delete snapshot;
        return CmdCode_OK;
    }

    PrintToServer("[SM] Apply all command \"%s\" noe exists.", command);
    return CmdCode_InvalidArg;
}

static CommandCode CmdExecute_List(DataPack data)
{
    #pragma unused data

    // Get all registered logger names
    StringMap loggers = new StringMap();
    Registry.Instance().ApplyAll(null, CB_OnApplyAll_GetNames, loggers);
    StringMapSnapshot snapshot = loggers.Snapshot();
    delete loggers;

    // https://github.com/alliedmodders/sourcemod/blob/1bc536fdfa0fe088300ae3532ede2da84b7a80d9/core/logic/smn_console.cpp#L91
    const int MAX_CONSOLE_LENGTH = 1024;
    char buffer[MAX_CONSOLE_LENGTH];
    for (int i = snapshot.Length - 1; i >= 0; --i)
    {
        int keyLen = snapshot.KeyBufferSize(i);
        char[] key = new char[keyLen + 1];
        snapshot.GetKey(i, key, keyLen + 1);

        if (i == 0) FormatEx(buffer, sizeof(buffer), "\"%s\"", key);
        else        Format(buffer, sizeof(buffer), "%s, \"%s\"", buffer, key);
    }
    delete snapshot;

    PrintToServer("[SM] List of all logger names: [%s].", buffer);
    return CmdCode_OK;
}

static CommandCode CmdExecute_Version(DataPack data)
{
    #pragma unused data

    char time[64], tags[256];
    int version = GetLog4spVersion(time, sizeof(time), tags, sizeof(tags));

    PrintToServer(" Log4sp version information:");
    PrintToServer("    Version: %u.%u.%u", (version >> 16) & 0xFF, (version >> 8) & 0xFF, version & 0xFF);
    PrintToServer("    Build time: %s", time);
    PrintToServer("    Build tags: %s", tags);
    PrintToServer("    Manager version: " ... PLUGIN_VERSION);
    PrintToServer("    Manager build time: " ... __DATE__ ... " " ... __TIME__);
    return CmdCode_OK;
}

static void CB_OnApplyAll_GetNames(Logger logger, StringMap loggers)
{
    int size = logger.GetNameLength() + 1;
    char[] name = new char[size];
    logger.GetName(name, size);
    loggers.SetValue(name, 1);
}


#undef COMMAND_MAX_LENGTH
#undef LOG4SP_LEVEL_NAMES

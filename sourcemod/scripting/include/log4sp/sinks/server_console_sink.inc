#if defined _log4sp_sinks_server_console_sink_included
    #endinput
#endif
#define _log4sp_sinks_server_console_sink_included

#include <log4sp/sinks/sink>


/**
 * Outputs log messages to the terminal or console, useful for real-time
 * debugging and development.
 */
methodmap ServerConsoleSink < Sink
{
#if !defined LOG4SP_HEADER_ONLY
    /**
     * This sink is output to the server console.
     *
     * @note ServerConsoleSink handles must be freed via delete or CloseHandle().
     *
     * @return          A new ServerConsoleSink Handle.
     */
    public native ServerConsoleSink();

    /**
     * Returns whether or not a server console sink handle is valid.
     *
     * @return          True if valid, false otherwise.
     */
    public native bool IsValid();

#else

    /**
     * This sink is output to the server console.
     *
     * @note ServerConsoleSink handles must be freed via Sink.Close().
     * @note Multi-plugin shared Handles must be freed before unload the plugin.
     *
     * @return          A new ServerConsoleSink Handle.
     */
    public ServerConsoleSink() {
        ServerConsoleSink sink = view_as<ServerConsoleSink>(Sink.__Sink("ServerConsoleSink",
                                                                        null, __ServerConsoleSink_OnLog,
                                                                        null, INVALID_FUNCTION,
                                                                        null, INVALID_FUNCTION));
        return sink;
    }

    public bool IsValid() {
        if (!view_as<Sink>(this).IsValid())
            return false;

        char classname[32];
        this.__GetClassname(classname, sizeof(classname));
        return StrEqual(classname, "ServerConsoleSink");
    }
#endif      // LOG4SP_HEADER_ONLY
}


#if defined LOG4SP_HEADER_ONLY
public bool __ServerConsoleSink_OnLog(Sink sink, char[] error, int maxlen, const char[] logTime, LOG4SP_ES_CONST SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg)
{
    int len = strlen(logTime);
    char seconds[21], millisecond[4];
    strcopy(seconds, len - 8, logTime);
    strcopy(millisecond, sizeof(millisecond), logTime[len - 9]);

    char date[9]; // 12:34:56
    FormatTime(date, sizeof(date), "%H:%M:%S", StringToInt(seconds)); // FIXME: Possible Year 2038 Problem

    char source[256];
    if (!loc.IsEmpty())
    {
        int sepOffset = 0;
        for (int i = 0; i < sizeof(SourceLoc::filename); ++i)
        {
            if (!loc.filename[i])
                break;
            if (loc.filename[i] == '\\' || loc.filename[i] == '/')
                sepOffset = i + 1;
        }
        FormatEx(source, sizeof(source), "[%s::%u] ", loc.filename[sepOffset], loc.line);
    }

    char level[2];
    LogLevelToShortName(level, sizeof(level), lvl);

    // https://github.com/alliedmodders/sourcemod/blob/1bc536fdfa0fe088300ae3532ede2da84b7a80d9/core/logic/smn_console.cpp#L91
    // Due to limited buffer size (1024), use short dates and levels.
    PrintToServer("[%s.%s] [%s] [%c] %s%s", date, millisecond, name, level[0], source, msg);
    return true;
}
#endif      // LOG4SP_HEADER_ONLY

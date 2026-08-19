#if defined _log4sp_sinks_client_console_all_sink_included
    #endinput
#endif
#define _log4sp_sinks_client_console_all_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/callback_sink>


methodmap ClientConsoleAllSink < CallbackSink
{
    /**
     * This sink is output to all client console.
     *
     * @return          A new ClientConsoleAllSink Handle.
     */
    public ClientConsoleAllSink() {
        StringMap data = new StringMap();
        data.SetString("m_szClassname", "ClientConsoleAllSink");
        return view_as<ClientConsoleAllSink>(new CallbackSink(null, __ClientConsoleAllSink_OnLog,
                                                              null, INVALID_FUNCTION,
                                                              null, __ClientConsoleAllSink_OnClose,
                                                              data));
    }

    /**
     * Returns whether or not a client console all sink handle is valid.
     *
     * @return          True if valid, false otherwise.
     */
    public bool IsValid() {
        CallbackSink sink = view_as<CallbackSink>(this);
        if (!sink.IsValid())
            return false;

        StringMap data = sink.GetData();
        char classname[32];
        data.GetString("m_szClassname", classname, sizeof(classname));
        return StrEqual(classname, "ClientConsoleAllSink");
    }
}


public bool __ClientConsoleAllSink_OnLog(Sink sink, char[] error, int maxlen, const char[] logTime, LOG4SP_ES_CONST SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg)
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

    // https://github.com/alliedmodders/sourcemod/blob/1bc536fdfa0fe088300ae3532ede2da84b7a80d9/core/logic/smn_console.cpp#L132
    // Due to limited buffer size (1024), use short dates and levels.
    // PrintToConsoleAll() only has a buffer size of 254 while
    // PrintToConsole() has a buffer size of 1024.
    for (int i = 1; i <= MaxClients; i++)
    {
        if (IsClientInGame(i))
        {
            SetGlobalTransTarget(i);
            PrintToConsole(i, "[%s.%s] [%s] [%c] %s%s", date, millisecond, name, level[0], source, msg);
        }
    }
    return true;
}

public void __ClientConsoleAllSink_OnClose(CallbackSink sink)
{
    delete view_as<StringMap>(sink.GetData());

    // It will be destroyed soon, so ignore resetting it to null.
    // sink.SetData(null);
}


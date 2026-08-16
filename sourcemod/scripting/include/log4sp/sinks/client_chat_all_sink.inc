#if defined _log4sp_sinks_client_chat_all_sink_included
    #endinput
#endif
#define _log4sp_sinks_client_chat_all_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/logger>
#include <log4sp/sinks/callback_sink>


methodmap ClientChatAllSink < CallbackSink
{
    /**
     * This sink is output to all client chat.
     *
     * @return          A new ClientChatAllSink Handle.
     */
    public ClientChatAllSink() {
        return view_as<ClientChatAllSink>(new CallbackSink(null, __ClientChatAllSink_OnLog,
                                                           null, INVALID_FUNCTION,
                                                           null, __ClientChatAllSink_OnClose));
    }
}


public bool __ClientChatAllSink_OnLog(Sink sink, char[] error, int maxlen, const char[] logTime, const SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg)
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

    // https://github.com/alliedmodders/sourcemod/blob/1bc536fdfa0fe088300ae3532ede2da84b7a80d9/core/smn_halflife.cpp#L332
    // Due to limited buffer size (254), use short dates and levels.
    PrintToChatAll("[%s.%s] [%s] [%c] %s%s", date, millisecond, name, level[0], source, msg);
    return true;
}

public void __ClientChatAllSink_OnClose(CallbackSink sink)
{
    delete view_as<StringMap>(sink.GetData());

    // It will be destroyed soon, so ignore resetting it to null.
    // sink.SetData(null);
}

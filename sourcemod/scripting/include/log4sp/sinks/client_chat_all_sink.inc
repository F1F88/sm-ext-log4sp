#if defined _log4sp_sinks_client_chat_all_sink_included
    #endinput
#endif
#define _log4sp_sinks_client_chat_all_sink_included

#include <log4sp/sinks/callback_sink>


methodmap ClientChatAllSink < CallbackSink
{
    /**
     * This sink is output to all client chat.
     *
     * @return          A new ClientChatAllSink Handle.
     */
    public ClientChatAllSink() {
        StringMap data = new StringMap();
        data.SetString("m_szClassname", "ClientChatAllSink");
        return view_as<ClientChatAllSink>(new CallbackSink(null, __ClientChatAllSink_OnLog,
                                                           null, INVALID_FUNCTION,
                                                           null, __ClientChatAllSink_OnClose,
                                                           data));
    }

    /**
     * Returns whether or not a client chat all sink handle is valid.
     *
     * @return          True if valid, false otherwise.
     */
    public bool IsValid() {
        CallbackSink sink = view_as<CallbackSink>(this);
        if (!sink.IsValid())
            return false;

        StringMap data = sink.GetData();
        if (!data)
            return false;

        char classname[32];
        data.GetString("m_szClassname", classname, sizeof(classname));
        return StrEqual(classname, "ClientChatAllSink");
    }
}


public bool __ClientChatAllSink_OnLog(Sink sink, char[] error, int maxlen, LOG4SP_ES_CONST Long logTime, LOG4SP_ES_CONST SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg)
{
    int secs, msec;
#if !defined SM_INT64_SUPPORTED
    {
        // Bitwise division DivideAndRetain performs poorly
        char buf[21];
        int len = logTime.ToString(buf, sizeof(buf));
        if (len < 3)
        {
            msec = logTime.Low();
            secs = 0;
        }
        else
        {
            msec = (buf[len - 3] - '0') * 100 + (buf[len - 2] - '0') * 10 + (buf[len - 1] - '0');
            buf[len - 3] = '\0';
            secs = StringToInt(buf);
        }
    }
#else
    {
        Long seconds, milliseconds;
        logTime.DivideAndRemainder(IntToLong(1000), seconds, milliseconds);
        msec = milliseconds.Low();
        secs = seconds.Low();
    }
#endif      // SM_INT64_SUPPORTED

    char date[9]; // 12:34:56
    FormatTime(date, sizeof(date), "%H:%M:%S", secs); // FIXME: Possible Year 2038 Problem

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
        FormatEx(source, sizeof(source), "[%s:%u] ", loc.filename[sepOffset], loc.line);
    }

    char level[2];
    LogLevelToShortName(level, sizeof(level), lvl);

    // https://github.com/alliedmodders/sourcemod/blob/1bc536fdfa0fe088300ae3532ede2da84b7a80d9/core/smn_halflife.cpp#L332
    // Due to limited buffer size (254), use short dates and levels.
    PrintToChatAll("[%s.%03d] [%s] [%c] %s%s", date, msec, name, level[0], source, msg);
    return true;
}

public void __ClientChatAllSink_OnClose(StringMap data)
{
    delete data;
}

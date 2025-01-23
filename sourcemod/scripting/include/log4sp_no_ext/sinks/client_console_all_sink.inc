#if defined _log_for_sourcepawn_sinks_client_console_all_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_client_console_all_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp_no_ext/sinks/callback_sink>


methodmap ClientConsoleAllSink < CallbackSink
{
    public ClientConsoleAllSink()
    {
        return null;
    }

    public static Logger CreateLogger(const char[] name) {
        return null;
    }
}

// static void __CBSink_PrintToClientConsoleAll(const char[] msg)
// {
//     PrintToConsoleAll(msg);
// }


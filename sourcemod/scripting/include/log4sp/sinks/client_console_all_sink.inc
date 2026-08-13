#if defined _log4sp_sinks_client_console_all_sink_included
 #endinput
#endif
#define _log4sp_sinks_client_console_all_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/logger>
#include <log4sp/sinks/callback_sink>


methodmap ClientConsoleAllSink < CallbackSink
{
    /**
     * This sink is output to all client console.
     *
     * @return          A new ClientConsoleAllSink Handle.
     */
    public ClientConsoleAllSink() {
        return view_as<ClientConsoleAllSink>(new CallbackSink(_, __CBSink_PrintToClientConsoleAll));
    }
}

static void __CBSink_PrintToClientConsoleAll(const char[] msg)
{
    PrintToConsoleAll(msg);
}


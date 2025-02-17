#if defined _log_for_sourcepawn_sinks_client_console_all_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_client_console_all_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/callback_sink>


methodmap ClientConsoleAllSink < CallbackSink
{
    /**
     * This sink is output to all client console.
     *
     * @return              ClientConsoleSink handle.
     */
    public ClientConsoleAllSink()
    {
        return view_as<ClientConsoleAllSink>(new CallbackSink(_, __CBSink_PrintToClientConsoleAll));
    }

    /**
     * Create a logger handle that outputs to all client console.
     *
     * @param name      The name of the new logger.
     * @return          Logger handle.
     * @error           Logger name already exits.
     */
    public static Logger CreateLogger(const char[] name)
    {
        Logger logger = new Logger(name);
        logger.AddSinkEx(new ClientConsoleAllSink());
        return logger;
    }
}

static void __CBSink_PrintToClientConsoleAll(const char[] msg)
{
    PrintToConsoleAll(msg);
}


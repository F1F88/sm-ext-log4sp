#if defined _log4sp_sinks_client_chat_all_sink_included
 #endinput
#endif
#define _log4sp_sinks_client_chat_all_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/callback_sink>


methodmap ClientChatAllSink < CallbackSink
{
    /**
     * This sink is output to all client chat.
     *
     * @return              ClientChatSink handle.
     */
    public ClientChatAllSink()
    {
        return view_as<ClientChatAllSink>(new CallbackSink(_, __CBSink_PrintToClientChatAll));
    }

    /**
     * Create a logger handle that outputs to all client chat.
     *
     * @param name      The name of the new logger.
     * @return          Logger handle.
     * @error           Logger name already exits.
     */
    public static Logger CreateLogger(const char[] name)
    {
        Logger logger = new Logger(name);
        logger.AddSinkEx(new ClientChatAllSink());
        return logger;
    }
}

static void __CBSink_PrintToClientChatAll(const char[] msg)
{
    PrintToChatAll(msg);
}

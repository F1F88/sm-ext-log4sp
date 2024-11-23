#if defined _log_for_sourcepawn_sinks_client_chat_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_client_chat_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp_empty/sinks/sink>


typeset ClientChatSinkFilter
{
    function Action (int client);
    function Action (int client, const char[] name, LogLevel lvl);
    function Action (int client, const char[] name, LogLevel lvl, const char[] msg);
};



methodmap ClientChatSink < Sink
{
    public ClientchatSinkST() { return view_as<ClientChatSink>(INVALID_HANDLE); }

    public void SetFilter(ClientChatSinkFilter filter) {}
}

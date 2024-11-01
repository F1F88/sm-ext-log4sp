#if defined _log_for_sourcepawn_sinks_client_console_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_client_console_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp_empty/sinks/sink>


typeset ClientConsoleSinkFilter
{
    function Action (int client);
    function Action (int client, const char[] name, LogLevel lvl);
    function Action (int client, const char[] name, LogLevel lvl, const char[] msg);
};



methodmap ClientConsoleSinkST < Sink
{
    public ClientConsoleSinkST() { return view_as<ClientConsoleSinkST>(INVALID_HANDLE); }

    public void SetFilter(ClientConsoleSinkFilter filter) {}
}


methodmap ClientConsoleSinkMT < Sink
{
    public ClientConsoleSinkMT() { return view_as<ClientConsoleSinkMT>(INVALID_HANDLE); }

    public void SetFilter(ClientConsoleSinkFilter filter) {}
}

#if defined _log_for_sourcepawn_sinks_server_console_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_server_console_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp_empty/sinks/sink>


methodmap ServerConsoleSinkST < Sink
{
    public ServerConsoleSinkST() { return view_as<ServerConsoleSinkST>(INVALID_HANDLE); }
}


methodmap ServerConsoleSinkMT < Sink
{
    public ServerConsoleSinkMT() { return view_as<ServerConsoleSinkMT>(INVALID_HANDLE); }
}

#if defined _log_for_sourcepawn_sinks_server_console_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_server_console_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp_empty/sinks/sink>


methodmap ServerConsoleSink < Sink
{
    public ServerConsoleSink(bool multiThread = false) {
        return view_as<ServerConsoleSink>(INVALID_HANDLE);
    }
}

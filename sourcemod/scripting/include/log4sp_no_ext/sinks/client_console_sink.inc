#if defined _log_for_sourcepawn_sinks_client_console_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_client_console_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp_empty/sinks/sink>


methodmap ClientConsoleSink < Sink
{
    public ClientConsoleSink(bool multiThread = false) {
        return view_as<ClientConsoleSink>(INVALID_HANDLE);
    }

    public void SetFilter(SinkClientFilter filter) {}
}

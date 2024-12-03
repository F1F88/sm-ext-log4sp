#if defined _log_for_sourcepawn_sinks_base_file_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_base_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp_empty/sinks/sink>


methodmap BaseFileSink < Sink
{
    public BaseFileSink(const char[] file, bool truncate = false, bool multiThread = false) {
        return view_as<BaseFileSink>(INVALID_HANDLE);
    }

    public void GetFilename(char[] buffer, int maxlen) {}
}
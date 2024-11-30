#if defined _log_for_sourcepawn_sinks_base_file_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_base_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp_empty/sinks/sink>


methodmap BaseFileSinkST < Sink
{
    public BaseFileSinkST(const char[] file, bool truncate = false) { return view_as<BaseFileSinkST>(INVALID_HANDLE); }

    public void GetFilename(char[] buffer, int maxlen) {}
}


methodmap BaseFileSinkMT < Sink
{
    public BaseFileSinkMT(const char[] file, bool truncate = false) { return view_as<BaseFileSinkMT>(INVALID_HANDLE); }

    public void GetFilename(char[] buffer, int maxlen) {}
}

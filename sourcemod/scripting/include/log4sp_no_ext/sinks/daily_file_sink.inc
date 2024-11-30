#if defined _log_for_sourcepawn_sinks_daily_file_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_daily_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp_empty/sinks/sink>


methodmap DailyFileSinkST < Sink
{
    public DailyFileSinkST(const char[] file, int rotationHour=0, int rotationMinute=0, bool truncate=false, int maxFiles=0) { return view_as<DailyFileSinkST>(INVALID_HANDLE); }

    public void GetFilename(char[] buffer, int maxlength) {}
}


methodmap DailyFileSinkMT < Sink
{
    public DailyFileSinkMT(const char[] file, int rotationHour=0, int rotationMinute=0, bool truncate=false, int maxFiles=0) { return view_as<DailyFileSinkMT>(INVALID_HANDLE); }

    public void GetFilename(char[] buffer, int maxlength) {}
}

#if defined _log_for_sourcepawn_sinks_daily_file_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_daily_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp_no_ext/sinks/sink>


methodmap DailyFileSink < Sink
{
    public DailyFileSink(const char[] file, int rotationHour = 0, int rotationMinute = 0, bool truncate = false, int maxFiles = 0) {
        return view_as<DailyFileSink>(INVALID_HANDLE);
    }

    public int GetFilename(char[] buffer, int maxlength) {}

    public Logger CreateLogger(const char[] name, const char[] file, int hour = 0, int minute = 0, bool truncate = false, int maxFiles = 0) {
        return null;
    }
}

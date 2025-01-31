#if defined _log_for_sourcepawn_sinks_daily_file_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_daily_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp_no_ext/sinks/sink>


typedef DailyFileCalculator = function void (char[] filename, int maxlen, const char[] timestampSec);


methodmap DailyFileSink < Sink
{
    public DailyFileSink(const char[] file, int hour = 0, int minute = 0, bool truncate = false, int maxFiles = 0, DailyFileCalculator callback = INVALID_FUNCTION) {
        return null;
    }

    public int GetFilename(char[] buffer, int maxlength) {}

    public Logger CreateLogger(const char[] name, const char[] file, int hour = 0, int minute = 0, bool truncate = false, int maxFiles = 0, DailyFileCalculator callback = INVALID_FUNCTION) {
        return null;
    }
}


stock void DailyFileFormatCalculator(char[] filename, int maxlen, const char[] timestampSec)
{
    FormatTime(filename, maxlen, filename, StringToInt(timestampSec));
}

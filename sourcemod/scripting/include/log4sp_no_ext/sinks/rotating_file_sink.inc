#if defined _log_for_sourcepawn_sinks_rotating_file_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_rotating_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp_empty/sinks/sink>


methodmap RotatingFileSink < Sink
{
    public RotatingFileSink(const char[] file, const int maxFileSize, const int maxFiles, bool rotateOnOpen = false, bool multiThread = false) {
        return view_as<RotatingFileSink>(INVALID_HANDLE);
    }

    public void GetFilename(char[] buffer, int maxlength) {}

    public static void CalcFilename(const char[] file, int index, char[] buffer, int maxlength) {}
}

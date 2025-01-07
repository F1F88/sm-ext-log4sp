#if defined _log_for_sourcepawn_sinks_daily_file_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_daily_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>


/**
 * Rotating file sink based on date.
 */
methodmap DailyFileSink < Sink
{
    /**
     * Create daily file sink which rotates on given time.
     *
     * @param file          The file path where the log messages will be written.
     * @param rotationHour  The hour of the day when the log file should be rotated. (0-23)
     * @param rotationMinute The minute of the hour when the log file should be rotated. (0-59)
     * @param truncate      If true, the created file will be truncated.
     * @param maxFiles      If max_files > 0, retain only the last max_files and delete previous.
     * @param multiThread   If true, the created sink will be multi threaded and thread safe.
     * @return              DailyFileSink handle.
     * @error               Invalid rotation time in ctor.
     */
    public native DailyFileSink(const char[] file,
                                int rotationHour = 0,
                                int rotationMinute = 0,
                                bool truncate = false,
                                int maxFiles = 0,
                                bool multiThread = false);

    /**
     * Get the current filename being used by the file sink.
     *
     * @param buffer        Buffer to store file name.
     * @param maxlen        Maximum length of the buffer.
     * @return              Number of characters written to the buffer, not including the null terminator.
     */
    public native int GetFilename(char[] buffer, int maxlength);
}

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
     * @return              DailyFileSink handle.
     * @error               Invalid rotation time in ctor.
     */
    public native DailyFileSink(const char[] file,
                                int rotationHour = 0,
                                int rotationMinute = 0,
                                bool truncate = false,
                                int maxFiles = 0);

    /**
     * Get the current filename being used by the file sink.
     *
     * @param buffer        Buffer to store file name.
     * @param maxlen        Maximum length of the buffer.
     * @return              Number of characters written to the buffer, not including the null terminator.
     */
    public native int GetFilename(char[] buffer, int maxlength);

    /**
     * Create a logger handle that outputs to a file and rotates the file based on date.
     *
     * @param name      The name of the logger.
     * @param file      The file path where the log messages will be written.
     * @param hour      The hour of the day when the log file should be rotated. (0-23)
     * @param minute    The minute of the hour when the log file should be rotated. (0-59)
     * @param truncate  If true, the created file will be truncated.
     * @param maxFiles  If max_files > 0, retain only the last max_files and delete previous.
     * @return          Logger handle.
     * @error           Logger name already exits, or invalid rotation time.
     */
    public static native Logger CreateLogger(
        const char[] name,
        const char[] file,
        int hour = 0,
        int minute = 0,
        bool truncate = false,
        int maxFiles = 0);
}

#if defined _log_for_sourcepawn_sinks_daily_file_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_daily_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>


/*
 * Rotating file sink based on date.
 * If truncate != false , the created file will be truncated.
 * If max_files > 0, retain only the last max_files and delete previous.
 */
methodmap DailyFileSinkST < Sink
{
    /**
     * Create daily file sink which rotates on given time.
     *
     * @param file          The file path where the log messages will be written.
     * @param rotationHour  The hour of the day when the log file should be rotated (0-23). Default is 0.
     * @param rotationMinute The minute of the hour when the log file should be rotated (0-59). Default is 0.
     * @param truncate      If truncate != false , the created file will be truncated.
     * @param maxFiles      If max_files > 0, retain only the last max_files and delete previous.
     * @return              Handle for a single-threaded daily file sink.
     * @error               Invalid rotation time in ctor, create sink handle failed, invalid rotation time in ctor.
     */
    public native DailyFileSinkST(
        const char[] file,
        int rotationHour = 0,
        int rotationMinute = 0,
        bool truncate = false,
        int maxFiles = 0
    );

    /**
     * Get the current filename being used by the file sink.
     *
     * @param buffer        Buffer to store file name.
     * @param maxlen        Maximum size of string buffer.
     * @error               Invalid sink handle.
     */
    public native void GetFilename(char[] buffer, int maxlength);
}

/*
 * Rotating file sink based on date.
 * If truncate != false , the created file will be truncated.
 * If max_files > 0, retain only the last max_files and delete previous.
 */
methodmap DailyFileSinkMT < Sink
{
    /**
     * Create daily file sink which rotates on given time.
     *
     * @param file          The file path where the log messages will be written.
     * @param rotationHour  The hour of the day when the log file should be rotated (0-23). Default is 0.
     * @param rotationMinute The minute of the hour when the log file should be rotated (0-59). Default is 0.
     * @param truncate      If truncate != false , the created file will be truncated.
     * @param maxFiles      If max_files > 0, retain only the last max_files and delete previous.
     * @return              Handle for a multi-threaded daily file sink.
     * @error               Invalid rotation time in ctor, create sink handle failed.
     */
    public native DailyFileSinkMT(
        const char[] file,
        int rotationHour = 0,
        int rotationMinute = 0,
        bool truncate = false,
        int maxFiles = 0
    );

    /**
     * Get the current filename being used by the file sink.
     *
     * @param buffer        Buffer to store file name.
     * @param maxlen        Maximum size of string buffer.
     * @error               Invalid sink handle.
     */
    public native void GetFilename(char[] buffer, int maxlength);
}

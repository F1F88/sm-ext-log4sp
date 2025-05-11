#if defined _log_for_sourcepawn_sinks_daily_file_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_daily_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>


/**
 * Callback function called when calculating the daily log file name.
 *
 * @param filename  Daily log file name.
 * @param maxlen    Maximum length of the filename.
 * @param sec       Timestamp in seconds.
 */
typeset DailyFileCalculator
{
    function void (char[] filename, int maxlen, int sec);
}


/**
 * Rotating file sink based on date.
 *
 * @note Note that old log files from previous executions will not be deleted by this class,
 *       rotation and deletion is only applied while the program is running.
 */
methodmap DailyFileSink < Sink
{
    /**
     * Create daily file sink which rotates on given time.
     *
     * @note Note that old log files from previous executions will not be deleted by this class,
     *       rotation and deletion is only applied while the program is running.
     *
     * @param file          The file path where the log messages will be written.
     * @param hour          The hour of the day when the log file should be rotated. (0-23)
     * @param minute        The minute of the hour when the log file should be rotated. (0-59)
     * @param truncate      If true, the created file will be truncated.
     * @param maxFiles      If max_files > 0, retain only the last max_files and delete previous.
     * @param calculator    Callback function called when calculating the daily log file name.
     * @param openPre       Called before the sink open the file.
     * @param closePost     Called after the sink close the file.
     * @return              DailyFileSink handle.
     * @error               Invalid rotation time in ctor.
     */
    public native DailyFileSink(const char[] file,
                                int hour = 0,
                                int minute = 0,
                                bool truncate = false,
                                int maxFiles = 0,
                                DailyFileCalculator calculator = INVALID_FUNCTION,
                                SinkFileOpenPre openPre = INVALID_FUNCTION,
                                SinkFileClosePost closePost = INVALID_FUNCTION);

    /**
     * Get the current filename being used by the file sink.
     *
     * @param buffer        Buffer to store file name.
     * @param maxlen        Maximum length of the buffer.
     * @return              Number of characters written to the buffer, not including the null terminator.
     */
    public native int GetFilename(char[] buffer, int maxlen);

    /**
     * Gets the length of the filename.
     *
     * @return              Length of the filename string.
     */
    public native int GetFilenameLength();

    /**
     * Create a logger handle that outputs to a file and rotates the file based on date.
     *
     * @param name          The name of the logger.
     * @param file          The file path where the log messages will be written.
     * @param hour          The hour of the day when the log file should be rotated. (0-23)
     * @param minute        The minute of the hour when the log file should be rotated. (0-59)
     * @param truncate      If true, the created file will be truncated.
     * @param maxFiles      If max_files > 0, retain only the last max_files and delete previous.
     * @param calculator    For generate log file names.
     * @param openPre       Called before the sink open the file.
     * @param closePost     Called after the sink close the file.
     * @return              Logger handle.
     * @error               Logger name already exits, or invalid rotation time.
     */
    public static native Logger CreateLogger(
        const char[] name,
        const char[] file,
        int hour = 0,
        int minute = 0,
        bool truncate = false,
        int maxFiles = 0,
        DailyFileCalculator callback = INVALID_FUNCTION,
        SinkFileOpenPre openPre = INVALID_FUNCTION,
        SinkFileClosePost closePost = INVALID_FUNCTION);
}


/**
 * Generator of daily log file names with strftime format.
 * Usages:
 *    DailyFileSink sink = new DailyFileSink("file_%Y-%m-%d.log", hour, minute, truncate,
 *          maxFiles, DailyFileFormatCalculator);
 *
 *    Logger logger = DailyFileSink.CreateLogger("name", "file_%Y-%m-%d.log", hour, minute,
 *          truncate, maxFiles, DailyFileFormatCalculator);
 */
stock void DailyFileFormatCalculator(char[] filename, int maxlen, int sec)
{
    // FormatTime buffer cannot be the same as the format
    char buffer[PLATFORM_MAX_PATH];
    strcopy(buffer, sizeof(buffer), filename);
    FormatTime(filename, maxlen, buffer, sec);
}

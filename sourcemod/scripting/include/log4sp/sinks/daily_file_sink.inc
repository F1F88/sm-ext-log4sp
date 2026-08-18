#if defined _log4sp_sinks_daily_file_sink_included
    #endinput
#endif
#define _log4sp_sinks_daily_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>


/**
 * Callback function called when calculating the daily log file name.
 *
 * @param filename  Daily log file name.
 * @param maxlen    Maximum length of the filename.
 * @param timestamp Timestamp in seconds.
 */
typedef DailyFileCalculator = function void (char[] filename, int maxlen, const char[] timestamp);


/**
 * Generator of daily log file names with strftime format.
 */
public void DailyFileFormatCalculator(char[] filename, int maxlen, const char[] timestamp) {
    // FormatTime buffer cannot be the same as the format
    char buffer[256];
    strcopy(buffer, sizeof(buffer), filename);
    FormatTime(filename, maxlen, buffer, StringToInt(timestamp)); // FIXME: Possible Year 2038 Problem
}


/**
 * Creates new log files daily at a specified time, helping organize logs by date
 * for the easier tracking and to the analysis sink files.
 *
 * @note Note that old log files from previous executions will not be deleted by
 *       this class, rotation and deletion is only applied while the program is
 *       running.
 */
methodmap DailyFileSink < Sink
{
    /**
     * Create daily file sink which rotates on given time.
     *
     * @note Note that old log files from previous executions will not be deleted by this class,
     *       rotation and deletion is only applied while the program is running.
     * @note DailyFileSink handles must be freed via delete or CloseHandle().
     *
     * @param baseFilename  The file path where the log messages will be written.
     * @param hour          The hour of the day when the log file should be rotated. (0-23)
     * @param minute        The minute of the hour when the log file should be rotated. (0-59)
     * @param truncate      If true, the created file will be truncated.
     * @param maxFiles      If max_files > 0, retain only the last max_files and delete previous.
     * @param calcPlugin    Handle of the plugin that contains the calculator function.
     *                      If null, defaults to the calling plugin.
     * @param calcFunc      Function to call when calculating the daily log file name.
     * @param openPlugin    Handle of the plugin that contains the file open function.
     *                      If null, defaults to the calling plugin.
     * @param openFunc      Function to call before the sink open the file.
     * @param closePlugin   Handle of the plugin that contains the file close function.
     *                      If null, defaults to the calling plugin.
     * @param closeFunc     Function to call after the sink close the file.
     * @return              A new DailyFileSink Handle.
     * @error               Invalid rotation time in ctor, or maxFiles < 0, or maxFiles > 65535.
     */
    public native DailyFileSink(const char[] baseFilename,
                                int hour=0,
                                int minute=0,
                                bool truncate=false,
                                int maxFiles=0,
                                Handle calcPlugin=null,  DailyFileCalculator calcFunc=INVALID_FUNCTION,
                                Handle openPlugin=null,  FileOpenFunction openFunc=INVALID_FUNCTION,
                                Handle closePlugin=null, FileCloseFunction closeFunc=INVALID_FUNCTION);

    /**
     * Get the current filename being used by the file sink.
     *
     * @param buffer        Buffer to store file name.
     * @param maxlen        Maximum length of the buffer.
     * @return              Number of bytes written.
     */
    public native int GetFilename(char[] buffer, int maxlen);
}

#if defined _log4sp_sinks_rotating_file_sink_included
 #endinput
#endif
#define _log4sp_sinks_rotating_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>


/**
 * Rotating file sink based on size.
 *
 * Rotate files:
 *  log.txt   -> log.1.txt
 *  log.1.txt -> log.2.txt
 *  log.2.txt -> log.3.txt
 *  log.3.txt -> delete
 */
methodmap RotatingFileSink < Sink
{
    /**
     * Rotating file sink based on size.
     *
     * @param file          The file path where the log messages will be written.
     * @param maxFileSize   The maximum size of the log file in bytes before it gets rotated.
     * @param maxFiles      The maximum number of rotated log files to keep.
     * @param rotateOnOpen  If true, the log file will be rotated when opened.
     * @param openPre       Called before the sink open the file.
     * @param closePost     Called after the sink close the file.
     * @return              RotatingFileSink handle.
     * @error               Param maxFileSize <= 0, Param maxFiles > 200000.
     */
    public native RotatingFileSink(const char[] file,
                                   const int maxFileSize,
                                   const int maxFiles,
                                   bool rotateOnOpen = false,
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
     * Rotate log files on demand.
     */
    public native void RotateNow();

    /**
     * Calc filename according to index and file extension if exists.
     * e.g. calcFilename("logs/mylog.txt, 3) => "logs/mylog.3.txt".
     *
     * @param buffer        Buffer to store file name.
     * @param maxlen        Maximum length of the buffer.
     * @param file          Original file path.
     * @param index         File index.
     * @return              Number of characters written to the buffer, not including the null terminator.
     */
    public static native int CalcFilename(char[] buffer, int maxlen, const char[] file, int index);

    /**
     * Create a logger handle that outputs to a file and rotates the file based on size.
     *
     * @param name          The name of the logger.
     * @param file          The file path where the log messages will be written.
     * @param maxFileSize   The maximum size of the log file in bytes before it gets rotated.
     * @param maxFiles      The maximum number of rotated log files to keep.
     * @param rotateOnOpen  If true, the log file will be rotated when opened.
     * @param openPre       Called before the sink open the file.
     * @param closePost     Called after the sink close the file.
     * @return              Logger handle.
     * @error               Logger name already exits, or maxFileSize == 0, or maxFiles > 200000.
     */
    public static native Logger CreateLogger(
        const char[] name,
        const char[] file,
        int maxFileSize,
        int maxFiles,
        bool rotateOnOpen = false,
        SinkFileOpenPre openPre = INVALID_FUNCTION,
        SinkFileClosePost closePost = INVALID_FUNCTION);
}

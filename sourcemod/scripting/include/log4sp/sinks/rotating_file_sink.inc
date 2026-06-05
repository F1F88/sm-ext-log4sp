#if defined _log4sp_sinks_rotating_file_sink_included
 #endinput
#endif
#define _log4sp_sinks_rotating_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/logger>
#include <log4sp/sinks/sink>


/**
 * Automatically rotates log files based on size limits, preventing files from
 * growing too large and ensuring better file management.
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
     * @note RotatingFileSink handles must be freed via delete or CloseHandle().
     *
     * @param file          The file path where the log messages will be written.
     * @param maxFileSize   The maximum size of the log file in bytes before it gets rotated.
     * @param maxFiles      The maximum number of rotated log files to keep.
     * @param rotateOnOpen  If true, the log file will be rotated when opened.
     * @param openPre       Function to call before the sink open the file.
     * @param closePost     Function to call after the sink close the file.
     * @return              A new RotatingFileSink Handle.
     * @error               Param maxFileSize <= 0, Param maxFiles > 200000.
     */
    public native RotatingFileSink(const char[] file,
                                   const int maxFileSize,
                                   const int maxFiles,
                                   bool rotateOnOpen=false,
                                   SinkFileOpenPre openPre=INVALID_FUNCTION,
                                   SinkFileClosePost closePost=INVALID_FUNCTION);

    /**
     * Get the current filename being used by the file sink.
     *
     * @param buffer        Buffer to store file name.
     * @param maxlen        Maximum length of the buffer.
     * @return              Number of bytes written.
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
     * @return              Number of bytes written.
     */
    public static native int CalcFilename(char[] buffer, int maxlen, const char[] file, int index);

    /**
     * Create a logger handle that outputs to a file and rotates the file based on size.
     *
     * @note Logger handles must be freed via delete or CloseHandle().
     *
     * @param name          The name of the new logger.
     * @param file          The file path where the log messages will be written.
     * @param maxFileSize   The maximum size of the log file in bytes before it gets rotated.
     * @param maxFiles      The maximum number of rotated log files to keep.
     * @param rotateOnOpen  If true, the log file will be rotated when opened.
     * @param openPre       Function to call before the sink open the file.
     * @param closePost     Function to call after the sink close the file.
     * @return              A new Logger Handle.
     * @error               Logger name already exists, or maxFileSize == 0, or maxFiles > 200000.
     */
    public static native Logger CreateLogger(
        const char[] name,
        const char[] file,
        int maxFileSize,
        int maxFiles,
        bool rotateOnOpen=false,
        SinkFileOpenPre openPre=INVALID_FUNCTION,
        SinkFileClosePost closePost=INVALID_FUNCTION);
}

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
     * @param baseFilename  The file path where the log messages will be written.
     * @param maxSize       The maximum size of the log file in bytes before it gets rotated.
     * @param maxFiles      The maximum number of rotated log files to keep.
     * @param rotateOnOpen  If true, the log file will be rotated when opened.
     * @param openPlugin    Handle of the plugin that contains the file open function.
     *                      If null, defaults to the calling plugin.
     * @param openFunc      Function to call before the sink open the file.
     * @param closePlugin   Handle of the plugin that contains the file close function.
     *                      If null, defaults to the calling plugin.
     * @param closeFunc     Function to call after the sink close the file.
     * @return              A new RotatingFileSink Handle.
     * @error               Param maxFile <= 0, Param maxFiles > 200000.
     */
    public native RotatingFileSink(const char[] baseFilename,
                                   const int maxSize,
                                   const int maxFiles,
                                   bool rotateOnOpen=false,
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

    /**
     * Rotate log files on demand.
     *
     * @param error         Error message buffer in case of rotate failure.
     * @param maxlen        Maximum length of the error.
     * @return              True on success, false on error.
     */
    public native bool RotateNow(char[] error="", int maxlen=0);

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
}

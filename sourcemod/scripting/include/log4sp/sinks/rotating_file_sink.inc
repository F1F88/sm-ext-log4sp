#if defined _log_for_sourcepawn_sinks_rotating_file_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_rotating_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>


methodmap RotatingFileSinkST < Sink
{
    /**
     * Rotating file sink based on size.
     * Rotate files:
     *      log.txt   -> log.1.txt
     *      log.1.txt -> log.2.txt
     *      log.2.txt -> log.3.txt
     *      log.3.txt -> delete
     *
     * @param name          The name of the logger. This is used to identify the logger instance.
     * @param file          The file path where the log messages will be written.
     * @param maxFileSize   The maximum size of the log file in bytes before it gets rotated.
     * @param maxFiles      The maximum number of rotated log files to keep.
     * @param rotateOnOpen  Whether to rotate the log file upon opening.
     * @return              Handle for a single-threaded rotating file sink.
     * @error               Param maxFileSize <= 0, Param maxFiles > 200000, create sink handle failed.
     */
    public native RotatingFileSinkST(
        const char[] file,
        const int maxFileSize,
        const int maxFiles,
        bool rotateOnOpen = false
    );

    /**
     * Get the current filename being used by the file sink.
     *
     * @param buffer        Buffer to store file name.
     * @param maxlen        Maximum size of string buffer.
     * @note                The native will log a message and:
     *                      error - If sink handle is invalid - Return false.
     */
    public native void GetFilename(char[] buffer, int maxlength);

    /**
     * Calc filename according to index and file extension if exists.
     * e.g. calcFilename("logs/mylog.txt, 3) => "logs/mylog.3.txt".
     *
     * @param file          Original file path.
     * @param index         File index.
     * @param buffer        Buffer to store file name.
     * @param maxlen        Maximum size of string buffer.
     * @error               Invalid sink handle.
     */
    public native void CalcFilename(const char[] file, int index, char[] buffer, int maxlength);
}


methodmap RotatingFileSinkMT < Sink
{
    /**
     * Rotating file sink based on size.
     * Rotate files:
     *      log.txt   -> log.1.txt
     *      log.1.txt -> log.2.txt
     *      log.2.txt -> log.3.txt
     *      log.3.txt -> delete
     *
     * @param name          The name of the logger. This is used to identify the logger instance.
     * @param file          The file path where the log messages will be written.
     * @param maxFileSize   The maximum size of the log file in bytes before it gets rotated.
     * @param maxFiles      The maximum number of rotated log files to keep.
     * @param rotateOnOpen  Whether to rotate the log file upon opening.
     * @return              Handle for a multi-threaded rotating file sink.
     * @error               Param maxFileSize <= 0, Param maxFiles > 200000, create sink handle failed.
     */
    public native RotatingFileSinkMT(
        const char[] file,
        const int maxFileSize,
        const int maxFiles,
        bool rotateOnOpen = false
    );

    /**
     * Get the current filename being used by the file sink.
     *
     * @param buffer        Buffer to store file name.
     * @param maxlen        Maximum size of string buffer.
     * @note                The native will log a message and:
     *                      error - If sink handle is invalid - Return false.
     */
    public native void GetFilename(char[] buffer, int maxlength);

    /**
     * Calc filename according to index and file extension if exists.
     * e.g. calcFilename("logs/mylog.txt, 3) => "logs/mylog.3.txt".
     *
     * @param file          Original file path.
     * @param index         File index.
     * @param buffer        Buffer to store file name.
     * @param maxlen        Maximum size of string buffer.
     * @error               Invalid sink handle.
     */
    public native void CalcFilename(const char[] file, int index, char[] buffer, int maxlength);
}

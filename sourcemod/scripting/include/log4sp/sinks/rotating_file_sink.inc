#if defined _log_for_sourcepawn_sinks_rotating_file_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_rotating_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>


/**
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
     * @param multiThread   If true, the created sink will be multi threaded and thread safe.
     * @return              RotatingFileSink handle.
     * @error               Param maxFileSize <= 0, Param maxFiles > 200000.
     */
    public native RotatingFileSink(const char[] file,
                                   const int maxFileSize,
                                   const int maxFiles,
                                   bool rotateOnOpen = false,
                                   bool multiThread = false);

    /**
     * Get the current filename being used by the file sink.
     *
     * @param buffer        Buffer to store file name.
     * @param maxlen        Maximum length of the buffer.
     */
    public native void GetFilename(char[] buffer, int maxlength);

    public native void RotateNow();

    /**
     * Calc filename according to index and file extension if exists.
     * e.g. calcFilename("logs/mylog.txt, 3) => "logs/mylog.3.txt".
     *
     * @param file          Original file path.
     * @param index         File index.
     * @param buffer        Buffer to store file name.
     * @param maxlen        Maximum length of the buffer.
     */
    public static native void CalcFilename(const char[] file, int index, char[] buffer, int maxlength);
}

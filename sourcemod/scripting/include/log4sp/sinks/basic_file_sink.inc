#if defined _log4sp_sinks_basic_file_sink_included
 #endinput
#endif
#define _log4sp_sinks_basic_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/logger>
#include <log4sp/sinks/sink>


/**
 * Trivial file sink with single file as target.
 */
methodmap BasicFileSink < Sink
{
    /**
     * Trivial file sink with single file as target.
     *
     * @param file      The file path where the log messages will be written.
     * @param truncate  If true, the created file will be truncated.
     * @param openPre   Called before the sink open the file.
     * @param closePost Called after the sink close the file.
     * @return          BasicFileSink handle.
     */
    public native BasicFileSink(const char[] file,
                                bool truncate = false,
                                SinkFileOpenPre openPre = INVALID_FUNCTION,
                                SinkFileClosePost closePost = INVALID_FUNCTION);

    /**
     * Get the current filename being used by the file sink.
     *
     * @param buffer    Buffer to store file name.
     * @param maxlen    Maximum length of the buffer.
     * @return          Number of characters written to the buffer, not including the null terminator.
     */
    public native int GetFilename(char[] buffer, int maxlen);

    /**
     * Truncate log files on demand.
     */
    public native void Truncate();

    /**
     * Create a logger handle that outputs to a file.
     *
     * @param name      The name of the new logger.
     * @param file      The file path where the log messages will be written.
     * @param truncate  If true, the created file will be truncated.
     * @param openPre   Called before the sink open the file.
     * @param closePost Called after the sink close the file.
     * @return          Logger handle.
     * @error           Logger name already exits.
     */
    public static native Logger CreateLogger(
        const char[] name,
        const char[] file,
        bool truncate = false,
        SinkFileOpenPre onOpen = INVALID_FUNCTION,
        SinkFileClosePost onClose = INVALID_FUNCTION);
}

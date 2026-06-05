#if defined _log4sp_sinks_basic_file_sink_included
 #endinput
#endif
#define _log4sp_sinks_basic_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/logger>
#include <log4sp/sinks/sink>


/**
 * Writes log messages directly to files with efficient performance, making it
 * ideal for persistent storage and long-term log tracking in applications.
 */
methodmap BasicFileSink < Sink
{
    /**
     * Trivial file sink with single file as target.
     *
     * @note BasicFileSink handles must be freed via delete or CloseHandle().
     *
     * @param file      The file path where the log messages will be written.
     * @param truncate  If true, the created file will be truncated.
     * @param openPre   Function to call before the sink open the file.
     * @param closePost Function to call after the sink close the file.
     * @return          A new BasicFileSink Handle.
     */
    public native BasicFileSink(const char[] file,
                                bool truncate=false,
                                SinkFileOpenPre openPre=INVALID_FUNCTION,
                                SinkFileClosePost closePost=INVALID_FUNCTION);

    /**
     * Get the current filename being used by the file sink.
     *
     * @param buffer    Buffer to store file name.
     * @param maxlen    Maximum length of the buffer.
     * @return          Number of bytes written.
     */
    public native int GetFilename(char[] buffer, int maxlen);

    /**
     * Truncate log files on demand.
     */
    public native void Truncate();

    /**
     * Create a logger handle that outputs to a file.
     *
     * @note Logger handles must be freed via delete or CloseHandle().
     *
     * @param name      The name of the new logger.
     * @param file      The file path where the log messages will be written.
     * @param truncate  If true, the created file will be truncated.
     * @param openPre   Function to call before the sink open the file.
     * @param closePost Function to call after the sink close the file.
     * @return          A new Logger Handle.
     * @error           Logger name already exists.
     */
    public static native Logger CreateLogger(
        const char[] name,
        const char[] file,
        bool truncate=false,
        SinkFileOpenPre onOpen=INVALID_FUNCTION,
        SinkFileClosePost onClose=INVALID_FUNCTION);
}

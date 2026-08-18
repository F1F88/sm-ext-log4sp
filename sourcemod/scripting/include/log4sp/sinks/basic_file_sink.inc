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
     * @param filename      The file path where the log messages will be written.
     * @param truncate      If true, the created file will be truncated.
     * @param openPlugin    Handle of the plugin that contains the file open function.
     *                      If null, defaults to the calling plugin.
     * @param openFunc      Function to call before the sink open the file.
     * @param closePlugin   Handle of the plugin that contains the file close function.
     *                      If null, defaults to the calling plugin.
     * @param closeFunc     Function to call after the sink close the file.
     * @return              A new BasicFileSink Handle.
     */
    public native BasicFileSink(const char[] filename,
                                bool truncate=false,
                                Handle openPlugin=null,  FileOpenFunction openFunc=INVALID_FUNCTION,
                                Handle closePlugin=null, FileCloseFunction closeFunc=INVALID_FUNCTION);

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
     *
     * @param error         Error message buffer in case of truncate failure.
     * @param maxlen        Maximum length of the error.
     * @return              True on success, false on error.
     */
    public native bool Truncate(char[] error="", int maxlen=0);
}
